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

#include "JobContext.h"

#include "Basics/Common.h"
#include "Hydra/Conductor.h"
#include "Hydra/Mailbox.h"
#include "Hydra/ChannelStore.h"
#include "Scheduler/Scheduler.h"
#include "Scheduler/SchedulerFeature.h"

using namespace arangodb;

thread_local hydra::JobContext* hydra::JobContext::current = nullptr;


hydra::JobContext::JobContext(JobId jid, velocypack::Slice const& params) : _id(jid), _serverId(""),
  _canceled(false), _params(params) {
  
  if (!hydra::algorithms::validate(params.slice())) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_BAD_PARAMETER);
  }
  _serverId = ServerState::instance()->id();
  _channels = new ChannelStore();
  
  if (ServerState::instance()->isCoordinator()) {
    // FIXME
    _coordinator = nullptr;
  } else if (ServerState::instance->isDBServer()) {
    VPackSlice coordinator = params.get("coordinator");
    if (!coordinator.isString()) {
      THROW_ARANGO_EXCEPTION(TRI_ERROR_BAD_PARAMETER);
    }
    _conductor = new ConductorConnection(coordinator.copyString());
  } else if (ServerState::instance->isSingleServer()) {
    _conductor = new ConductorImpl();
  }
  _mailbox = new Mailbox(this);
}

hydra::JobContext::~JobContext() {
  delete _channels;
  delete _coordinator;
  delete _mailbox;
}

hydra::JobContext::start() {
  if (SchedulerFeature::SCHEDULER->isStopping()) {
    return; // shutdown ongoing
  }
  
  ServerState::Role role = ServerState::instance()->role();
  if (ServerState::isDBServer(role) ||
      ServerState::isSingleServer(role)) {
    auto func = hydra::algorithms::resolve(params.slice());
    TRI_ASSERT(func); // algorithms::validate prevents this
    if (!func) {
      THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_BAD_PARAMETER,
                                     "algorithm not found");
    }
    
    rest::Scheduler* scheduler = SchedulerFeature::SCHEDULER;
    scheduler->post([this, func] {
      JobContext::current = this;
      TRI_DEFER(JobContext::current = nullptr);
      func(_params.slice());
    });
    _conductor->notifyStartedJob(_serverId);
  } else if (ServerState::isCoordinator(role)) {
    _conductor->startJob();
  }
}


hydra::JobContext::cancel() {
  if (SchedulerFeature::SCHEDULER->isStopping()) {
    return; // shutdown ongoing
  }
  
  _canceled = true;
  
  
}
