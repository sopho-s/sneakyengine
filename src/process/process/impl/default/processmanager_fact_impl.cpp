#include "processmanager_fact_impl.hpp"
#include "processmanager_impl.hpp"

#include <memory>

namespace process {

  std::unique_ptr<IProcessManager>
  ProcessManagerFactoryImpl::create_processmanager() {
    return std::make_unique<process_internal::ProcessManagerImpl>();
  }
}