#include <storages/postgres/tests/util_test.hpp>

#include <gtest/gtest.h>

#include <utest/utest.hpp>

#include <storages/postgres/cluster.hpp>
#include <storages/postgres/detail/connection.hpp>
#include <storages/postgres/dsn.hpp>
#include <storages/postgres/exceptions.hpp>

namespace pg = storages::postgres;

namespace {

void CheckTransaction(pg::Transaction trx) {
  pg::ResultSet res{nullptr};

  // TODO Check idle connection count before and after begin
  EXPECT_NO_THROW(res = trx.Execute("select 1"));
  EXPECT_TRUE((bool)res) << "Result set is obtained";
  // TODO Check idle connection count before and after commit
  EXPECT_NO_THROW(trx.Commit());
  EXPECT_THROW(trx.Commit(), pg::NotInTransaction);
  EXPECT_NO_THROW(trx.Rollback());
}

pg::Cluster CreateCluster(const std::string& dsn,
                          engine::TaskProcessor& bg_task_processor,
                          size_t max_size) {
  return pg::Cluster(pg::ClusterDescription(dsn), bg_task_processor, 0,
                     max_size);
}

}  // namespace

class PostgreCluster : public PostgreSQLBase,
                       public ::testing::WithParamInterface<std::string> {
  void ReadParam() override { dsn_ = GetParam(); }

 protected:
  std::string dsn_;
};

INSTANTIATE_TEST_CASE_P(
    /*empty*/, PostgreCluster, ::testing::ValuesIn(GetDsnFromEnv()),
    DsnToString);

TEST_P(PostgreCluster, ClusterSyncSlaveRW) {
  RunInCoro([this] {
    auto cluster = CreateCluster(dsn_, GetTaskProcessor(), 1);

    EXPECT_THROW(
        cluster.Begin(pg::ClusterHostType::kSyncSlave, pg::Transaction::RW),
        pg::ClusterUnavailable);
  });
}

TEST_P(PostgreCluster, ClusterAsyncSlaveRW) {
  RunInCoro([this] {
    auto cluster = CreateCluster(dsn_, GetTaskProcessor(), 1);

    EXPECT_THROW(
        cluster.Begin(pg::ClusterHostType::kSlave, pg::Transaction::RW),
        pg::ClusterUnavailable);
  });
}

TEST_P(PostgreCluster, ClusterEmptyPool) {
  RunInCoro([this] {
    auto cluster = CreateCluster(dsn_, GetTaskProcessor(), 0);

    EXPECT_THROW(cluster.Begin({}), pg::PoolError);
  });
}

TEST_P(PostgreCluster, ClusterTransaction) {
  RunInCoro([this] {
    auto cluster = CreateCluster(dsn_, GetTaskProcessor(), 1);

    CheckTransaction(cluster.Begin({}));
  });
}
