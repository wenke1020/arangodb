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

#include "Conductor.h"
#include "Basics/Common.h"
#include "Cluster/ClusterComm.h"
#include "Cluster/ServerState.h"
#include "Hydra/JobContext.h"
#include "VocBase/ticks.h"
#include "VocBase/vocbase.h"

using namespace arangodb;

hydra::ConductorConnection::ConductorConnection(JobContext* jc, std::string const& conductor)
  : ConductorBase(jc), _conductorHost(conductor) {
    TRI_ASSERT(jc);
  }

void hydra::ConductorConnection::createJob(velocypack::Slice const&) {
  TRI_ASSERT(ServerState::instance()->isRunningInCluster());
  TRI_ASSERT(false); // should not be called here
}

//virtual void notifyJobCreated(std::string const& serverId) = 0;
void hydra::ConductorConnection::startJob() {
  TRI_ASSERT(ServerState::instance()->isDBServer());
  _job->start();
}
//virtual void notifyStartedJob(std::string const& serverId) = 0;
void hydra::ConductorConnection::cancelJob() {
  TRI_ASSERT(ServerState::instance()->isDBServer());
  _job->cancel(); // should call notifyCanceledJob afterwards
}

void hydra::ConductorConnection::notifyCanceledJob(std::string const& serverId) {
  TRI_ASSERT(ServerState::instance()->isDBServer());
  
  VPackBuilder builder;
  builder.openObject();
  builder.add("sender", VPackValue(_job->serverId()));
  builder.close();

  std::shared_ptr<ClusterComm> cc = ClusterComm::instance();
  CoordTransactionID trxID = TRI_NewTickServer();
  std::string path =  _job->apiPrefix() + "notify/canceled?job=" + std::to_string(_job->id());
  auto headers = std::make_unique<std::unordered_map<std::string, std::string>>();
  auto body = std::make_shared<std::string const>(builder.toJson());
  
  cc->asyncRequest("", trxID, "server:" + _conductorHost,
                   rest::RequestType::POST, path, body, headers, /*callback*/nullptr,
                   /*timeout*/120.0, true);  // single request, no answer expected
}

// =============================

hydra::ConductorImpl::ConductorImpl(JobContext* jc) : ConductorBase(jc) {}


void hydra::ConductorImpl::createJob(velocypack::Slice const&) {
  
}

//virtual void notifyJobCreated(std::string const& serverId) = 0;
void hydra::ConductorImpl::startJob() {
  
}
//virtual void notifyStartedJob(std::string const& serverId) = 0;
void hydra::ConductorImpl::cancelJob() {
  
}

void hydra::ConductorImpl::notifyCanceledJob(std::string const& serverId) {
  
}


