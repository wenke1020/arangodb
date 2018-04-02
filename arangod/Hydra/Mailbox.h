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

#include "Basics/Common.h"
#include "Basics/ConditionVariable.h"
#include "Hydra/Common.h"

#include <velocypack/Buffer.h>
#include <velocypack/Slice.h>

namespace arangodb {
namespace hydra {
  
  class JobContext;

/// Acts as a receiver for messages and data send by other workers
/// Channels can poll the mailbox for their input. The mailbox should
/// abstract away all communicationdetails
class Mailbox {
public:
  Mailbox(JobContext*);
  
  void receive(ChannelId, ChannelProgress, velocypack::Slice const&);
  void receiveCompleted(ChannelId, ChannelProgress);

  void send(std::string const& dst, ChannelId, ChannelProgress,
            velocypack::Slice const&);
  void sendCompleted(std::string const& dst, ChannelId, ChannelProgress);
  
  void poll(ChannelId, ChannelProgress, double timeout);
  
private:
  
  struct ChannelState {
    ChannelState() : _progress(0) {}
    
    /// current progress generation
    std::atomic<ChannelProgress> _progress;
    /// actual data received for this progress generation
    std::unordered_map<size_t, velocypack::Buffer<uint8_t>> _buffers;
    /// number of completed server responses per progress
    std::unordered_map<ChannelProgress, int> _completedCount;
    /// is current progress finished
    std::vector<bool> _progressDone;
  };
  
  JobContext* _context;
  
  basics::ConditionVariable _condition;
    std::unordered_map<ChannelId, ChannelState> _channels;
};
  
} // namespace hydra
}
#endif
