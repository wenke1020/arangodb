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

#include <string>
#include <velocypack/Slice.h>

namespace arangodb {
namespace hydra {

  class JobContext;
  
/// Inteface to hide cluster communication with the
/// coordinator with
class ConductorBase {
 public:
  
  ConductorBase(JobContext* jc) : _job(jc) {}
  virtual ~ConductorBase() {}
  
  //virtual void sendBroadcast(velocypack::Slice const&) = 0;
  
  //virtual void notifyStartJob(velocypack::Slice const&) = 0;
  virtual void createJob(velocypack::Slice const&) = 0;
  //virtual void notifyJobCreated(std::string const& serverId) = 0;
  virtual void startJob() = 0;
  //virtual void notifyStartedJob(std::string const& serverId) = 0;
  virtual void cancelJob() = 0;
  virtual void notifyCanceledJob(std::string const& serverId) = 0;
protected:
  JobContext* _job;
};
  
class ConductorConnection final : public ConductorBase {
 public:
  
  ConductorConnection(JobContext*,std::string const& server);
  
  void createJob(velocypack::Slice const&) override;
  //virtual void notifyJobCreated(std::string const& serverId) = 0;
  void startJob() override;
  //virtual void notifyStartedJob(std::string const& serverId) = 0;
  void cancelJob() override;
  void notifyCanceledJob(std::string const& serverId) override;
  
private:
  std::string _conductorHost;
};
  
/// Actual conductor implementation,
/// needs to track the number of responses
class ConductorImpl final : public ConductorBase {
public:
  
  ConductorImpl(JobContext*);
  
  void createJob(velocypack::Slice const&) override;
  //virtual void notifyJobCreated(std::string const& serverId) = 0;
  void startJob() override;
  //virtual void notifyStartedJob(std::string const& serverId) = 0;
  void cancelJob() override;
  void notifyCanceledJob(std::string const& serverId) override;
  
private:
  std::vector<std::string> _stoppedServers;
};

}  // namespace hydra
}
#endif
