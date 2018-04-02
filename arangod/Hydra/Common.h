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

#ifndef ARANGODB_HYDRA_COMMON_H
#define ARANGODB_HYDRA_COMMON_H 1


#if defined(__clang__) || defined(__GNUG__) || defined(_MSC_VER)
#define HYDRA_CHANNEL_ID  __COUNTER__
#else
#error Unsupported compiler
#endif

namespace arangodb {
namespace hydra {

  typedef uint64_t ChannelId;
  typedef uint64_t ChannelProgress;
  typedef uint64_t JobId;
  
  
}
}
#endif
