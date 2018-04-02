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

#ifndef ARANGODB_HYDRA_SHARDING_H
#define ARANGODB_HYDRA_SHARDING_H 1

#include <cstdint>
#include <string>
#include <vector>
#include <map>

struct TRI_vocbase_t;

namespace arangodb {
class LogicalCollection;
namespace hydra {

/// Sharding interface to handle sharding in a transparent way
class ShardingBase {
 public:
  ShardingBase() = default;
  virtual ~ShardingBase() = default;
  
  template<typename KeyT>
  std::string lookupTarget(KeyT const& key) const {
    lookupTargetInternal(&key, sizeof(KeyT));
  }
  
protected:
  virtual std::string const& lookupTargetInternal(void const* ptr, size_t len) const = 0;
};
  
/// sharding after a primary key
class PrimaryKeySharding final : public ShardingBase {
public:
  PrimaryKeySharding(TRI_vocbase_t* vocbase, std::string const& cname);
  PrimaryKeySharding(LogicalCollection*);
private:
   std::string const& lookupTargetInternal(void const* ptr, size_t len) const override;
private:
  std::shared_ptr<std::vector<std::string>> _shards;
  std::unordered_map<std::string, std::string> _servers;
};
  
/// Sharding
class SimpleSharding final : public ShardingBase {
public:
  SimpleSharding();
  std::string const& lookupTargetInternal(void const* ptr, size_t len) const override;
private:
  uint64_t const _seed;
};

}  // namespace hydra
}  // namespace arangodb
#endif
