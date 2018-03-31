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

#ifndef ARANGODB_HYDRA_COORDINATOR_H
#define ARANGODB_HYDRA_COORDINATOR_H 1

namespace arangodb {
namespace hydra {

  class JobContext;
  
/// Inteface to hide cluster communication with the
/// coordinator with
class ConductorBase {
 public:
  
  virtual ~CoordinatorBase() {}
  
  //virtual void sendBroadcast(velocypack::Slice const&) = 0;
  
  //virtual void notifyStartJob(velocypack::Slice const&) = 0;
  virtual void createJob(velocypack::Slice const&) = 0;
  //virtual void notifyJobCreated(std::string const& serverId) = 0;
  virtual void startJob() = 0;
  //virtual void notifyStartedJob(std::string const& serverId) = 0;
  virtual void cancelJob() = 0;
  virtual void notifyCanceledJob(std::string const& serverId) = 0;
};
  
class ConductorConnection : public ConductorBase {
 public:
  
  ConductorConnection(std::string const& server);
  
private:
  std::string _coordinatorHost;
  
};
  
/// Actual conductor implementation,
/// needs to track the number of responses
class ConductorImpl : public ConductorBase {
public:
  
  ConductorConnection(JobContext*);
  
private:
  JobContext* _job;
  std::vector<std::string> _stoppedServers;
};

}  // namespace hydra
}
