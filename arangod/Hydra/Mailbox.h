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

#ifndef ARANGODB_HYDRA_CHANNEL_BASE_H
#define ARANGODB_HYDRA_CHANNEL_BASE_H 1

#include <velocypack/velocypack-aliases.h>
#include <algorithm>
#include "Basics/Common.h"
#include "Basics/Mutex.h"


namespace arangodb {
namespace hydra {

/// Acts as a receiver for messages send by other workers
/// Channels can poll the mailbox for their input
class Mailbox {
  
  void push(ChannelId, velocypack::Slice const&);
  
  void poll(std::vector<ChannelId> const& channels,
            double timeout);
    
  protected:
    std::unordered_map<ChannelId,std::queue<velocypack::Buffer<uint8_t>>> _inbuffers;
  };
} // namespace hydra
}
#endif
