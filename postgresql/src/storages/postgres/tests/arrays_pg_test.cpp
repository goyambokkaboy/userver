#include <storages/postgres/io/array_types.hpp>
#include <storages/postgres/io/user_types.hpp>
#include <storages/postgres/tests/test_buffers.hpp>
#include <storages/postgres/tests/util_test.hpp>

namespace pg = storages::postgres;
namespace io = pg::io;
namespace tt = io::traits;

namespace static_test {

using one_dim_vector = std::vector<int>;
using two_dim_vector = std::vector<one_dim_vector>;
using three_dim_vector = std::vector<two_dim_vector>;

constexpr std::size_t kDimOne = 3;
constexpr std::size_t kDimTwo = 2;
constexpr std::size_t kDimThree = 1;

using one_dim_array = std::array<int, kDimOne>;
using two_dim_array = std::array<one_dim_array, kDimTwo>;
using three_dim_array = std::array<two_dim_array, kDimThree>;

using vector_of_arrays = std::vector<two_dim_array>;

static_assert(!tt::kIsCompatibleContainer<int>, "");

static_assert(tt::kIsCompatibleContainer<one_dim_vector>, "");
static_assert(tt::kIsCompatibleContainer<two_dim_vector>, "");
static_assert(tt::kIsCompatibleContainer<three_dim_vector>, "");

static_assert(tt::kIsCompatibleContainer<one_dim_array>, "");
static_assert(tt::kIsCompatibleContainer<two_dim_array>, "");
static_assert(tt::kIsCompatibleContainer<three_dim_array>, "");

static_assert(tt::kIsCompatibleContainer<vector_of_arrays>, "");

static_assert(tt::kDimensionCount<one_dim_vector> == 1, "");
static_assert(tt::kDimensionCount<two_dim_vector> == 2, "");
static_assert(tt::kDimensionCount<three_dim_vector> == 3, "");

static_assert(tt::kDimensionCount<one_dim_array> == 1, "");
static_assert(tt::kDimensionCount<two_dim_array> == 2, "");
static_assert(tt::kDimensionCount<three_dim_array> == 3, "");

static_assert(tt::kDimensionCount<vector_of_arrays> == 3, "");

static_assert(
    (std::is_same<tt::ContainerFinalElement<one_dim_vector>::type, int>::value),
    "");
static_assert(
    (std::is_same<tt::ContainerFinalElement<two_dim_vector>::type, int>::value),
    "");
static_assert((std::is_same<tt::ContainerFinalElement<three_dim_vector>::type,
                            int>::value),
              "");

static_assert(
    (std::is_same<tt::ContainerFinalElement<one_dim_array>::type, int>::value),
    "");
static_assert(
    (std::is_same<tt::ContainerFinalElement<two_dim_array>::type, int>::value),
    "");
static_assert((std::is_same<tt::ContainerFinalElement<three_dim_array>::type,
                            int>::value),
              "");

static_assert((std::is_same<tt::ContainerFinalElement<vector_of_arrays>::type,
                            int>::value),
              "");

static_assert(!tt::kHasFixedDimensions<one_dim_vector>, "");
static_assert(!tt::kHasFixedDimensions<two_dim_vector>, "");
static_assert(!tt::kHasFixedDimensions<three_dim_vector>, "");

static_assert(tt::kHasFixedDimensions<one_dim_array>, "");
static_assert(tt::kHasFixedDimensions<two_dim_array>, "");
static_assert(tt::kHasFixedDimensions<three_dim_array>, "");

static_assert(!tt::kHasFixedDimensions<vector_of_arrays>, "");

static_assert((std::is_same<std::integer_sequence<std::size_t, kDimOne>,
                            tt::FixedDimensions<one_dim_array>::type>::value),
              "");
static_assert(
    (std::is_same<std::integer_sequence<std::size_t, kDimTwo, kDimOne>,
                  tt::FixedDimensions<two_dim_array>::type>::value),
    "");
static_assert((std::is_same<std::integer_sequence<std::size_t, kDimThree,
                                                  kDimTwo, kDimOne>,
                            tt::FixedDimensions<three_dim_array>::type>::value),
              "");

static_assert(tt::kIsMappedToPg<one_dim_vector>, "");
static_assert(tt::kIsMappedToPg<two_dim_vector>, "");
static_assert(tt::kIsMappedToPg<three_dim_vector>, "");

static_assert(tt::kIsMappedToPg<one_dim_array>, "");
static_assert(tt::kIsMappedToPg<two_dim_array>, "");
static_assert(tt::kIsMappedToPg<three_dim_array>, "");

static_assert(tt::kIsMappedToPg<vector_of_arrays>, "");

}  // namespace static_test

namespace {

const pg::UserTypes types;

const std::string kArraysSQL = R"~(
select  '{1, 2, 3, 4}'::integer[],
        '{{1}, {2}, {3}, {4}}'::integer[],
        '{{1, 2}, {3, 4}}'::integer[],
        '{{{1}, {2}}, {{3}, {4}}}'::integer[],
        '{1, 2}'::smallint[],
        '{1, 2}'::bigint[]
)~";

TEST(PostgreIO, Arrays) {
  {
    static_test::one_dim_vector src{1, 2, 3};
    pg::test::Buffer buffer;
    EXPECT_NO_THROW(
        io::WriteBuffer<io::DataFormat::kBinaryDataFormat>(types, buffer, src));
    EXPECT_FALSE(buffer.empty());
    // PrintBuffer(buffer);
    auto fb =
        pg::test::MakeFieldBuffer(buffer, io::DataFormat::kBinaryDataFormat);
    static_test::one_dim_vector tgt;
    EXPECT_NO_THROW(io::ReadBuffer<io::DataFormat::kBinaryDataFormat>(fb, tgt));
    EXPECT_EQ(src, tgt);

    static_test::one_dim_array a1;
    EXPECT_NO_THROW(io::ReadBuffer<io::DataFormat::kBinaryDataFormat>(fb, a1));

    static_test::two_dim_array a2;
    EXPECT_THROW(io::ReadBuffer<io::DataFormat::kBinaryDataFormat>(fb, a2),
                 pg::DimensionMismatch);

    static_test::three_dim_array a3;
    EXPECT_THROW(io::ReadBuffer<io::DataFormat::kBinaryDataFormat>(fb, a3),
                 pg::DimensionMismatch);
  }
  {
    static_test::two_dim_vector src{{1, 2, 3}, {4, 5, 6}};
    pg::test::Buffer buffer;
    EXPECT_NO_THROW(
        io::WriteBuffer<io::DataFormat::kBinaryDataFormat>(types, buffer, src));
    EXPECT_FALSE(buffer.empty());
    // PrintBuffer(buffer);
    auto fb =
        pg::test::MakeFieldBuffer(buffer, io::DataFormat::kBinaryDataFormat);
    static_test::two_dim_vector tgt;
    EXPECT_NO_THROW(io::ReadBuffer<io::DataFormat::kBinaryDataFormat>(fb, tgt));
    EXPECT_EQ(src, tgt);

    static_test::two_dim_array a2;
    EXPECT_NO_THROW(io::ReadBuffer<io::DataFormat::kBinaryDataFormat>(fb, a2));

    static_test::one_dim_array a1;
    EXPECT_THROW(io::ReadBuffer<io::DataFormat::kBinaryDataFormat>(fb, a1),
                 pg::DimensionMismatch);
    static_test::three_dim_array a3;
    EXPECT_THROW(io::ReadBuffer<io::DataFormat::kBinaryDataFormat>(fb, a3),
                 pg::DimensionMismatch);
  }
  {
    using test_array = static_test::three_dim_array;
    test_array src{{1, 2, 3, 4, 5, 6}};
    pg::test::Buffer buffer;
    EXPECT_NO_THROW(
        io::WriteBuffer<io::DataFormat::kBinaryDataFormat>(types, buffer, src));
    EXPECT_FALSE(buffer.empty());
    // PrintBuffer(buffer);
    auto fb =
        pg::test::MakeFieldBuffer(buffer, io::DataFormat::kBinaryDataFormat);
    test_array tgt;
    EXPECT_NO_THROW(io::ReadBuffer<io::DataFormat::kBinaryDataFormat>(fb, tgt));
    EXPECT_EQ(src, tgt);
  }
  {
    using test_array = static_test::vector_of_arrays;
    test_array src{{1, 2, 3, 4, 5, 6}, {1, 2, 3, 4, 5, 6}};
    pg::test::Buffer buffer;
    EXPECT_NO_THROW(
        io::WriteBuffer<io::DataFormat::kBinaryDataFormat>(types, buffer, src));
    EXPECT_FALSE(buffer.empty());
    auto fb =
        pg::test::MakeFieldBuffer(buffer, io::DataFormat::kBinaryDataFormat);
    test_array tgt;
    EXPECT_NO_THROW(io::ReadBuffer<io::DataFormat::kBinaryDataFormat>(fb, tgt));
    EXPECT_EQ(src, tgt);
  }
  {
    /*! [Invalid dimensions] */
    static_test::two_dim_vector src{{1, 2, 3}, {4, 5}};
    pg::test::Buffer buffer;
    EXPECT_THROW(
        io::WriteBuffer<io::DataFormat::kBinaryDataFormat>(types, buffer, src),
        pg::InvalidDimensions);
    /*! [Invalid dimensions] */
  }
}

POSTGRE_TEST_P(ArrayRoundtrip) {
  ASSERT_TRUE(conn.get()) << "Expected non-empty connection pointer";
  pg::ResultSet res{nullptr};
  {
    using test_array = static_test::one_dim_vector;
    test_array src{-3, -2, 0, 1, 2, 3};
    EXPECT_NO_THROW(res = conn->Execute("select $1 as int_array", src));
    test_array tgt;
    EXPECT_NO_THROW(res[0][0].To(tgt));
    EXPECT_EQ(src, tgt);
  }
  {
    using test_array = static_test::vector_of_arrays;
    test_array src{{1, 2, 3, 4, 5, 6}, {1, 2, 3, 4, 5, 6}};
    EXPECT_NO_THROW(res = conn->Execute("select $1 as array3d", src));
    test_array tgt;
    EXPECT_NO_THROW(res[0][0].To(tgt));
    EXPECT_EQ(src, tgt);
  }
  {
    using test_array = std::vector<float>;
    test_array src{-3, -2, 0, 1, 2, 3};
    EXPECT_NO_THROW(res = conn->Execute("select $1 as float_array", src));
    test_array tgt;
    EXPECT_NO_THROW(res[0][0].To(tgt));
    EXPECT_EQ(src, tgt);
  }
  {
    using test_array = std::vector<std::string>;
    test_array src{"", "foo", "bar", ""};
    EXPECT_NO_THROW(res = conn->Execute("select $1 as text_array", src));
    test_array tgt;
    EXPECT_NO_THROW(res[0][0].To(tgt));
    EXPECT_EQ(src, tgt);
  }
  {
    using nullable_type = boost::optional<std::string>;
    using test_array = std::vector<nullable_type>;
    test_array src{
        {}, nullable_type{"foo"}, nullable_type{"bar"}, nullable_type{""}};
    EXPECT_NO_THROW(
        res = conn->Execute("select $1 as text_array_with_nulls", src));
    test_array tgt;
    EXPECT_NO_THROW(res[0][0].To(tgt));
    ASSERT_EQ(4, tgt.size());
    EXPECT_FALSE(tgt[0].is_initialized());
    EXPECT_TRUE(tgt[1].is_initialized());
    EXPECT_TRUE(tgt[2].is_initialized());
    EXPECT_TRUE(tgt[3].is_initialized());
    EXPECT_EQ(src, tgt);
    std::vector<std::string> tgt2;
    EXPECT_THROW(res[0][0].To(tgt2), pg::TypeCannotBeNull);
  }
}

}  // namespace
