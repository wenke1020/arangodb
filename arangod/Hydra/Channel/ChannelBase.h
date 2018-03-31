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

#if defined(__clang__) || defined(__GNUG__) || defined(_MSC_VER)
#define HYDRA_CHANNEL_ID  __COUNTER__
#else
#error Unsupported compiler
#endif

namespace arangodb {
namespace hydra {

  class Mailbox;
  class ShardingBase;
  
  typedef uint64_t ChannelId;
  typedef uint64_t ChannelProgress;

  class ChannelBase {
  public:
    enum class ChannelType { Sync, Async };
    
    virtual ~ChannelBase() = default;
    
    /// Getter
    inline ChannelId id() const { return _channelId; }
    inline ChannelType channelType() const { return _type; }
    inline ChannelProgress progress() const { return _progress; }

    /*void set_as_async_channel();
    void set_as_sync_channel();*/
    
    /// prepare() needs to be invoked to do some preparation work (if any), such as clearing buffers,
    /// before it can take new incoming communication using the in(BinStream&) method.
    /// In list_execute (in core/executor.hpp), prepare() is usually used before any in(BinStream&).
    virtual void prepare() = 0;
    
    /// in(Bufffer const&) defines what the channel should do when receiving a binstream
    virtual void in(velocypack::Buffer<uint8_t> const& bin) = 0;
    
    /// out() defines what the channel should do after a list_execute, normally mailbox->send_complete() will be invoked
    virtual void out() = 0
    
  protected:
    ChannelBase(ShardingBase* sharding);
    
    ChannelBase(const ChannelBase&) = delete;
    ChannelBase& operator=(const ChannelBase&) = delete;
    
    ChannelBase(ChannelBase&&) = default;
    ChannelBase& operator=(ChannelBase&&) = default;
    
    ChannelId _channelId;
    ChannelType _type;
    ChannelProgress _progress;
    
    Mailbox* _mailbox = nullptr;
    ShardingBase* _sharding;
  };
  
}
}
#endif
