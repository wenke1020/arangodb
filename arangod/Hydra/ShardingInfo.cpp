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


#include "ShardingInfo.h"
#include "Basics/hashes.h"
#include "Basics/VelocyPackHelper.h"
#include "Cluster/ClusterInfo.h"
#include "Cluster/ServerState.h"
#include "VocBase/LogicalCollection.h"
#include "Hydra/JobContext.h"

#include <velocypack/velocypack-aliases.h>

using namespace arangodb;

hydra::PrimaryKeySharding::PrimaryKeySharding(TRI_vocbase_t* vocbase, std::string const& cname) {
  if (ServerState::instance()->isRunningInCluster()) {
    ClusterInfo* ci = ClusterInfo::instance();
    auto coll = ci->getCollection(vocbase->name(), cname);
    _shards = ci->getShardList(std::to_string(coll->planId()));
    TRI_ASSERT(_shards);
    for (std::string const& shard : *_shards) {
      auto servers = ClusterInfo::instance()->getResponsibleServer(shard);
      if (servers->size() > 0) {
        _servers.emplace_back(shard, servers->at(0));
#ifdef ARANGODB_ENABLE_MAINTAINER_MODE
        auto workers = hydra::JobContext::current()->workers();
        TRI_ASSERT(std::find(workers.begin(), workers.end(),
                             servers->at(0)) != workers.end());
#endif
      }
    }
  } // not relevant outside the cluster
}

hydra::PrimaryKeySharding::PrimaryKeySharding(LogicalCollection* coll) {
  if (ServerState::instance()->isRunningInCluster()) {
    ClusterInfo* ci = ClusterInfo::instance();
    _shards = ci->getShardList(std::to_string(coll->planId()));
  } // not relevant outside the cluster
}

// c&p from resolveResponsibleShard
std::string const& hydra::PrimaryKeySharding::lookupTargetInternal(void const* ptr, size_t len) const {
  TRI_ASSERT(memchr(static_cast<const char*>(ptr), '/', len) == nullptr); // no _id allowed
  if (_shards->empty()) {
    return StaticStrings::Empty;
  }
  
  uint64_t hash = TRI_FnvHashBlockInitial();
  
#warning FIXME stop allocating the builder every time
  VPackBuilder temporaryBuilder;
  temporaryBuilder.add(VPackValuePair(reinterpret_cast<const char*>(ptr),
                                      len, VPackValueType::String));
  VPackSlice tmp = temporaryBuilder.slice();
  hash = tmp.normalizedHash(hash);

  static char const* magicPhrase =
  "Foxx you have stolen the goose, give she back again!";
  static size_t const magicPhraseLen = 52;
  // To improve our hash function:
  hash = TRI_FnvHashBlock(hash, magicPhrase, magicPhraseLen);
  
  std::string const& shard = _shards->at(hash % _shards->size());
  return _servers.at(shard);
}

hydra::SimpleSharding::SimpleSharding() : _seed(TRI_FnvHashBlockInitial()) {}

std::string const& hydra::SimpleSharding::lookupTargetInternal(void const* ptr, size_t len) const {
  uint64_t hash = TRI_FnvHashBlockInitial();
  hash = TRI_FnvHashBlock(hash, ptr, len);
  auto workers = hydra::JobContext::current()->workers();
  return workers[hash % workers.size()];
}
