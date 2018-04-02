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

#ifndef ARANGODB_HYDRA_JOB_CONTEXT_H
#define ARANGODB_HYDRA_JOB_CONTEXT_H 1

#include <cstdint>
#include <velocypack/Builder.h>
#include "Hydra/Common.h"

struct TRI_vocbase_t;
namespace arangodb {
namespace hydra {
  
class ConductorBase;
class Mailbox;
  
class JobContext final {
public:
  
  JobContext(JobId jid, TRI_vocbase_t*,
             velocypack::Slice const& params);
  ~JobContext();
  
  inline JobId id() const { return _id; }
  inline bool canceled() const { return _canceled; }
  inline velocypack::Slice params() const { return _params.slice(); }
  
  inline TRI_vocbase_t* vocbase() const { return _vocbase; }
  inline std::string const& serverId() const { return _serverId; }
  inline std::vector<std::string> const& workers() const { return _workers; }
  
  inline ConductorBase* conductor() const { return _conductor; }
  inline Mailbox* mailbox() const { return _mailbox; }
  
  std::string apiPrefix() const;
  
  void start();
  void cancel();
  
  static inline JobContext* current() { return hydra::JobContext::CONTEXT; }
  
  //inline std::string const& controllerHost();
  
private:
  JobId const _id;
  bool _canceled;
  velocypack::Builder _params;
  
  TRI_vocbase_t* _vocbase;
  std::string _serverId;
  std::vector<std::string> _workers;
  
  ConductorBase* _conductor;
  Mailbox* _mailbox;
  
  static thread_local JobContext* CONTEXT;
};

}  // namespace hydra
}  // namespace arangodb
#endif
