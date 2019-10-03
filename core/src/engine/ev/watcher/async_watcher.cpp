#include "async_watcher.hpp"

#include <engine/async.hpp>

namespace engine {
namespace ev {

AsyncWatcher::AsyncWatcher(ThreadControl& thread_control, Callback cb)
    : ev_async_(thread_control, this), cb_(std::move(cb)) {
  ev_async_.Init(&AsyncWatcher::OnEvent);
}

AsyncWatcher::~AsyncWatcher() = default;

void AsyncWatcher::Start() { ev_async_.Start(); }

// NOLINTNEXTLINE(bugprone-exception-escape)
void AsyncWatcher::OnEvent(struct ev_loop*, ev_async* async,
                           int events) noexcept {
  auto self = static_cast<AsyncWatcher*>(async->data);
  self->ev_async_.Stop();

  if (events & EV_ASYNC) {
    try {
      self->CallCb();
    } catch (const std::exception& ex) {
      LOG_ERROR() << "Uncaught exception in AsyncWatcher callback: " << ex;
    }
  }
}

void AsyncWatcher::Send() { ev_async_.Send(); }

void AsyncWatcher::CallCb() {
  LOG_DEBUG() << "CallCb (1) watcher=" << reinterpret_cast<long>(this);

  cb_();
}

}  // namespace ev
}  // namespace engine
