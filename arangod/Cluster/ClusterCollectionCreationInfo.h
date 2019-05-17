////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2019-2019 ArangoDB GmbH, Cologne, Germany
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
/// @author Michael Hackstein
////////////////////////////////////////////////////////////////////////////////

#ifndef ARANGOD_CLUSTER_CLUSTER_COLLECTION_CREATION_INFO_H
#define ARANGOD_CLUSTER_CLUSTER_COLLECTION_CREATION__INFO_H 1

#include "Basics/Common.h"

#include "Basics/StaticStrings.h"
#include "Basics/VelocyPackHelper.h"

#include <velocypack/Slice.h>

namespace arangodb {

struct ClusterCollectionCreationInfo {
  enum State { INIT, FAILED, DONE };
  ClusterCollectionCreationInfo(std::string const cID, uint64_t shards, uint64_t repFac,
                                bool waitForRep, velocypack::Slice const& slice)
      : collectionID(std::move(cID)),
        numberOfShards(shards),
        replicationFactor(repFac),
        waitForReplication(waitForRep),
        json(slice),
        name(arangodb::basics::VelocyPackHelper::getStringValue(
            json, arangodb::StaticStrings::DataSourceName, StaticStrings::Empty)),
        state(State::INIT) {
    if (numberOfShards == 0 || arangodb::basics::VelocyPackHelper::getBooleanValue(
                                   json, arangodb::StaticStrings::IsSmart, false)) {
      // Nothing to do this cannot fail
      state = State::DONE;
    }
    TRI_ASSERT(!name.empty());
  }

  std::string const collectionID;
  uint64_t numberOfShards;
  uint64_t replicationFactor;
  bool waitForReplication;
  velocypack::Slice const json;
  std::string name;
  std::function<bool(velocypack::Slice const& result)> dbServerChanged;
  State state;
};
}  // namespace arangodb

#endif