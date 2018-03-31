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

#ifndef ARANGODB_HYDRA_JOB_BASE_H
#define ARANGODB_HYDRA_JOB_BASE_H 1

#include <cstdint>
#include <velocypack/Builder.h>

namespace arangodb {
namespace hydra {
  
  class ChannelStore;
  class ConductorBase;
  class Mailbox;
  
typedef uint64_t JobId;
  
class JobContext final {
public:
  
  JobContext(JobId jid, velocypack::Slice const& params);
  ~JobContext();
  
  inline JobId id() const { return _id; }
  inline bool canceled() const { return _canceled; }
  inline velocypack::Slice const& params() const { _params.slice(); }
  inline Coordinator* coordinator() const { return _coordinator; }
  inline Mailbox* mailbox() const { return _mailbox; }
  
  void start();
  void cancel();
  
  
  size_t numWorkerMachines() const;
  
  static inline JobContext* current() { return JobContext::current; }
  
  //inline std::string const& controllerHost();
  
private:
  JobId const _id;
  std::string _serverId;
  bool _canceled;
  velocypack::Builder _params;
  
  ChannelStore* _channels;
  ConductorBase* _conductor;
  Mailbox* _mailbox;
  
  static thread_local JobContext* current;
};

}  // namespace hydra
}  // namespace arangodb
