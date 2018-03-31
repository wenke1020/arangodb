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

#include "Mailbox.h"

#include "Basics/ConditionLocker.h"
#include "Cluster/ClusterComm.h"
#include <velocypack/velocypack-aliases.h>

using namespace arangodb;

hydra::Mailbox::Mailbox(JobContext* ctx) : _context(ctx) {}

void hydra::Mailbox::receive(ChannelId cid, ChannelProgress prg,
                             VPackSlice const& data) {
  CONDITION_LOCK(guard, _condition);
  
  ChannelReceiverState& state = _channels[cid];
  auto it = state._buffers.find(prg);
  if(it == state._buffers.end()) {
    state._buffers.emplace(prg, VPackBuffer(data));
  } else {
    state._buffers[state._progress].appen
  }
}

void hydra::Mailbox::receiveCompleted(ChannelId cid, ChannelProgress progress,
                                      VPackSlice const& data) {
  CONDITION_LOCK(guard, _condition);
  
  ChannelReceiverState& state = _channels[cid];
  state._completedCount[progress] += 1;
  if (_context->numProcesses - 1 == state._completedCount[prg]) {
    if (state._progressDone.size() <= progress) {
      state._progressDone.resize(prg+1);
    }
    state._progressDone[progress] = true;
    guard.broadcast();
  }
}

void hydra::Mailbox::send(std::string const& dst, ChannelId cid, ChannelProgress progress,
                          velocypack::Slice const& data) {
  VPackBuilder builder;
  builder.openArray(true);
  builder.add(cid);
  builder.add(progress);
  builder.add(data);
  builder.close();
  
  ClusterComm::instance();
}

void hydra::Mailbox::sendCompleted(std::string const& dst, ChannelId cid,
                                   ChannelProgress progress) {
  VPackBuilder builder;
  builder.openArray(true);
  builder.add(cid);
  builder.add(progress);
  builder.close();
  
  ClusterComm::instance();

}
  
void hydra::Mailbox::poll(ChannelId cid, ChannelProgress prg, double timeout) {
  CONDITION_LOCK(guard, _condition);
  
  auto it = _channels.find(prg);
  while (it == state.end()) {
    guard.wait();
    it = state._buffers.find(prg);
  }
  
  ChannelState& state = it->second;
  while(!state._progressDone[prg]) {
    guard.wait();
  }
}

