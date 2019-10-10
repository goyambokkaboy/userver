#pragma once

#include <functional>
#include <vector>

#include <boost/optional.hpp>

#include <cache/lru_cache.hpp>
#include <engine/mutex.hpp>

namespace cache {

template <typename T, typename U, typename Hash = std::hash<T>,
          typename Equal = std::equal_to<T>>
class NWayLRU final {
 public:
  NWayLRU(size_t ways, size_t way_size, const Hash& hash = Hash(),
          const Equal& equal = Equal());

  void Put(const T& key, U value);

  template <typename Validator>
  boost::optional<U> Get(const T& key, Validator validator);

  boost::optional<U> Get(const T& key) {
    return Get(key, [](const U&) { return true; });
  }

  U GetOr(const T& key, const U& default_value);

  void Invalidate();

  void InvalidateByKey(const T& key);

  /// Iterates over all items. May be slow for big caches.
  template <typename Function>
  void VisitAll(Function func) const;

  size_t GetSize() const;

  void UpdateWaySize(size_t way_size);

 private:
  struct Way {
    Way(const Way& other) : cache(other.cache) {}

    // max_size is not used, will be reset by Resize() in NWayLRU::NWayLRU
    Way(const Hash& hash, const Equal& equal) : cache(1, hash, equal) {}

    mutable engine::Mutex mutex;
    LRU<T, U, Hash, Equal> cache;
  };

  Way& GetWay(const T& key);

  std::vector<Way> caches_;
  Hash hash_fn_;
};

template <typename T, typename U, typename Hash, typename Eq>
NWayLRU<T, U, Hash, Eq>::NWayLRU(size_t ways, size_t way_size, const Hash& hash,
                                 const Eq& equal)
    : caches_(ways, Way(hash, equal)), hash_fn_(hash) {
  if (ways == 0) throw std::logic_error("Ways must be positive");

  for (auto& way : caches_) way.cache.SetMaxSize(way_size);
}

template <typename T, typename U, typename Hash, typename Eq>
void NWayLRU<T, U, Hash, Eq>::Put(const T& key, U value) {
  auto& way = GetWay(key);

  std::unique_lock<engine::Mutex> lock(way.mutex);
  way.cache.Put(key, std::move(value));
}

template <typename T, typename U, typename Hash, typename Eq>
template <typename Validator>
boost::optional<U> NWayLRU<T, U, Hash, Eq>::Get(const T& key,
                                                Validator validator) {
  auto& way = GetWay(key);
  std::unique_lock<engine::Mutex> lock(way.mutex);
  auto* value = way.cache.Get(key);

  if (value) {
    if (validator(*value)) return *value;
    way.cache.Erase(key);
  }

  return boost::none;
}

template <typename T, typename U, typename Hash, typename Eq>
void NWayLRU<T, U, Hash, Eq>::InvalidateByKey(const T& key) {
  auto& way = GetWay(key);
  std::unique_lock<engine::Mutex> lock(way.mutex);
  way.cache.Erase(key);
}

template <typename T, typename U, typename Hash, typename Eq>
U NWayLRU<T, U, Hash, Eq>::GetOr(const T& key, const U& default_value) {
  auto& way = GetWay(key);
  std::unique_lock<engine::Mutex> lock(way.mutex);
  return way.cache.GetOr(key, default_value);
}

template <typename T, typename U, typename Hash, typename Eq>
void NWayLRU<T, U, Hash, Eq>::Invalidate() {
  for (auto& way : caches_) {
    std::unique_lock<engine::Mutex> lock(way.mutex);
    way.cache.Invalidate();
  }
}

template <typename T, typename U, typename Hash, typename Eq>
template <typename Function>
void NWayLRU<T, U, Hash, Eq>::VisitAll(Function func) const {
  for (const auto& way : caches_) {
    std::unique_lock<engine::Mutex> lock(way.mutex);
    way.cache.VisitAll(func);
  }
}

template <typename T, typename U, typename Hash, typename Eq>
size_t NWayLRU<T, U, Hash, Eq>::GetSize() const {
  size_t size{0};
  for (const auto& way : caches_) {
    std::unique_lock<engine::Mutex> lock(way.mutex);
    size += way.cache.GetSize();
  }
  return size;
}

template <typename T, typename U, typename Hash, typename Eq>
void NWayLRU<T, U, Hash, Eq>::UpdateWaySize(size_t way_size) {
  for (auto& way : caches_) {
    std::unique_lock<engine::Mutex> lock(way.mutex);
    way.cache.SetMaxSize(way_size);
  }
}

template <typename T, typename U, typename Hash, typename Eq>
typename NWayLRU<T, U, Hash, Eq>::Way& NWayLRU<T, U, Hash, Eq>::GetWay(
    const T& key) {
  auto n = hash_fn_(key) % caches_.size();
  return caches_[n];
}

}  // namespace cache
