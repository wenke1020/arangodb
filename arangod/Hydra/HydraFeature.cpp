////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2018 ArangoDB GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Simon Grätzer
////////////////////////////////////////////////////////////////////////////////

#include "HydraFeature.h"
#include <atomic>
#include "ApplicationFeatures/ApplicationServer.h"
#include "Basics/MutexLocker.h"
#include "Cluster/ClusterFeature.h"
#include "Cluster/ClusterInfo.h"
#include "Cluster/ServerState.h"
#include "Scheduler/Scheduler.h"
#include "Scheduler/SchedulerFeature.h"

using namespace arangodb;
using namespace arangodb::pregel;

static HydraFeature* Instance = nullptr;
static std::atomic<uint64_t> _uniqueId;

HydraFeature::HydraFeature(application_features::ApplicationServer* server)
    : application_features::ApplicationFeature(server, "Hydra") {
  setOptional(true);
  requiresElevatedPrivileges(false);
  startsAfter("WorkMonitor");
  startsAfter("Logger");
  startsAfter("Database");
  startsAfter("Endpoint");
  startsAfter("Cluster");
  startsAfter("Server");
}

HydraFeature::HydraFeature() {
  if (_recoveryManager) {
    _recoveryManager.reset();
  }
  cleanupAll();
}

HydraFeature* HydraFeature::instance() {
  return Instance;
}

size_t HydraFeature::availableParallelism() {
  const size_t procNum = TRI_numberProcessors();
  return procNum < 1 ? 1 : procNum;
}

void HydraFeature::start() {
  Instance = this;
  if (ServerState::instance()->isAgent()) {
    return;
  }

  // const size_t threadNum = PregelFeature::availableParallelism();
  // LOG_TOPIC(DEBUG, Logger::PREGEL) << "Pregel uses " << threadNum << "
  // threads";
  //_threadPool.reset(new ThreadPool(threadNum, "Pregel"));

  if (HydraFeature::instance()->isCoordinator()) {

  }
}

void HydraFeature::beginShutdown() {
  cleanupAll();
  Instance = nullptr;
}

void HydraFeature::addJob(std::unique_ptr<JobBase>&& job) {
  MUTEX_LOCKER(guard, _mutex);
  _jobs.emplace(job->id(), std::move(job));
}

JobBase* HydraFeature::job(uint64_t executionNumber) {
  if (SchedulerFeature::SCHEDULER->isStopping()) {
    return nullptr;
  }
  MUTEX_LOCKER(guard, _mutex);
  auto it = _jobs.find(executionNumber);
  return it != _jobs.end() ? it->second : nullptr;
}

void HydraFeature::cleanupJob(uint64_t executionNumber) {
  // unmapping etc might need a few seconds
  TRI_ASSERT(SchedulerFeature::SCHEDULER != nullptr);
  rest::Scheduler* scheduler = SchedulerFeature::SCHEDULER;
  scheduler->post([this, executionNumber] {
    MUTEX_LOCKER(guard, _mutex);

    auto wit = _jobs.find(executionNumber);
    if (wit != _jobs.end()) {
      _jobs.erase(executionNumber);
    }
  });
}

void HydraFeature::cleanupAll() {
  MUTEX_LOCKER(guard, _mutex);
  _jobs.clear();
  /*for (auto it : _workers) {
    it.second->cancelGlobalStep(VPackSlice());
  }*/
}
/*
void HydraFeature::handleControlRequest(uint64_t jobId,
                                         std::string const& path,
                                         VPackSlice const& body) {
  if (SchedulerFeature::SCHEDULER->isStopping()) {
    return; // shutdown ongoing
  }
  
  auto jit = _jobs.find(jobId);
  if (jit == _jobs.end()) {
    THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_INTERNAL, "job not found");
  }

}

void HydraFeature::handleMailboxRequest(uint64_t jobId, uint64_t channel,
                                         velocypack::Slice const& data) {
  if (SchedulerFeature::SCHEDULER->isStopping()) {
    return; // shutdown ongoing
  }
  
  auto jit = _jobs.find(jobId);
  if (jit == _jobs.end()) {
    THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_INTERNAL, "job not found");
  }
  
  jit->second->mailbox()
}*/
