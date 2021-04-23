#include <components/minimal_component_list.hpp>
#include <components/run.hpp>
#include <fs/blocking/temp_directory.hpp>  // for fs::blocking::TempDirectory
#include <fs/blocking/write.hpp>  // for fs::blocking::RewriteFileContents

#include <components/component_list_test.hpp>
#include <utest/utest.hpp>

namespace {

const auto kTmpDir = fs::blocking::TempDirectory::Create();
const std::string kRuntimeConfingPath =
    kTmpDir.GetPath() + "/runtime_config.json";
const std::string kConfigVariablesPath =
    kTmpDir.GetPath() + "/config_vars.json";

// TODO: purge userver-cache-dump-path after TAXICOMMON-3540
const std::string kConfigVariables =
    fmt::format(R"(
  userver-cache-dump-path: {0}
  runtime_config_path: {1})",
                kTmpDir.GetPath(), kRuntimeConfingPath);

// BEWARE! No separate fs-task-processor. Testing almost single thread mode
const std::string kStaticConfig = R"(
components_manager:
  coro_pool:
    initial_size: 50
    max_size: 500
  default_task_processor: main-task-processor
  event_thread_pool:
    threads: 1
  task_processors:
    main-task-processor:
      thread_name: main-worker
      worker_threads: 1
  components:
    manager-controller:  # Nothing
    logging:
      fs-task-processor: main-task-processor
      loggers:
        default:
          file_path: '@null'
    tracer:
        service-name: config-service
    statistics-storage:
      # Nothing
    taxi-config:
      bootstrap-path: $runtime_config_path
      fs-cache-path: $runtime_config_path  # May differ from bootstrap-path
      fs-task-processor: main-task-processor
config_vars: )" + kConfigVariablesPath +
                                  R"(
)";

}  // namespace

TEST(CommonComponentList, Minimal) {
  tests::LogLevelGuard guard;

  fs::blocking::RewriteFileContents(kRuntimeConfingPath, tests::kRuntimeConfig);
  fs::blocking::RewriteFileContents(kConfigVariablesPath, kConfigVariables);

  components::RunOnce(components::InMemoryConfig{kStaticConfig},
                      components::MinimalComponentList());
}