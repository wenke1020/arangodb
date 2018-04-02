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
#include "Cluster/ServerState.h"
#include "Hydra/Algorithms.h"
#include "Hydra/Conductor.h"
#include "Hydra/Mailbox.h"
#include "Scheduler/Scheduler.h"
#include "Scheduler/SchedulerFeature.h"
#include "VocBase/vocbase.h"

#include "Logger/Logger.h"

using namespace arangodb;

hydra::JobContext::JobContext(JobId jid, TRI_vocbase_t* vocbase, velocypack::Slice const& params)
: _id(jid), _canceled(false), _params(params), _vocbase(vocbase), _serverId("")
    {
  
  VPackSlice algo = params.get("algorithm");
  if (!hydra::algorithms::validate(algo)) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_BAD_PARAMETER);
  }
  _serverId = ServerState::instance()->getId();
  
  if (ServerState::instance()->isSingleServerOrCoordinator()) {
    _conductor = new ConductorImpl(this);
  } else if (ServerState::instance()->isDBServer()) {
    VPackSlice coordinator = params.get("coordinator");
    if (!coordinator.isString()) {
      THROW_ARANGO_EXCEPTION(TRI_ERROR_BAD_PARAMETER);
    }
    _conductor = new ConductorConnection(this, coordinator.copyString());
  } else {
    TRI_ASSERT(false);
  }
  _mailbox = new Mailbox(this);
}

hydra::JobContext::~JobContext() {
  delete _conductor;
  delete _mailbox;
}

std::string hydra::JobContext::apiPrefix() const {
  return "/_db/" + basics::StringUtils::urlEncode(_vocbase->name()) + "/hydra/";
}

void hydra::JobContext::start() {
  if (SchedulerFeature::SCHEDULER->isStopping()) {
    return; // shutdown ongoing
  }
  
  ServerState::RoleEnum role = ServerState::instance()->getRole();
  if (ServerState::isDBServer(role) ||
      ServerState::isSingleServer(role)) {
    auto func = hydra::algorithms::resolve(_params.slice());
    TRI_ASSERT(func); // algorithms::validate prevents this
    if (!func) {
      THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_BAD_PARAMETER,
                                     "algorithm not found");
    }
    
    rest::Scheduler* scheduler = SchedulerFeature::SCHEDULER;
    scheduler->post([this, func] {
      JobContext::CONTEXT = this;
      TRI_DEFER(JobContext::CONTEXT = nullptr);
      if (!_canceled) {
        try {
          func(_params.slice());
        } catch(basics::Exception const& ex) {
          LOG_TOPIC(WARN, Logger::FIXME) << "Error during excecution: " << ex.what();
        }
      }
    });
    //_conductor-> notifyStartedJob(_serverId);
  } else if (ServerState::isCoordinator(role)) {
    _conductor->startJob();
  }
}


void hydra::JobContext::cancel() {
  if (SchedulerFeature::SCHEDULER->isStopping()) {
    return; // shutdown ongoing
  }
  _canceled = true;
}
