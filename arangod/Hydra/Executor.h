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

#include "Hydra/InputFormat.h"
#include "Hydra/DataFrame.h"

#include <cstdint>
#include <velocypack/Builder.h>

namespace arangodb {
namespace hydra {
  
namespace execute {
  
  template<typename InFmtT, >
  ObjList<InFmtT> loadInput(InputSource<InFmtT> const& list) {
    
  }
  
  template<typename InFmtT, typename ParseT, typename ObjT>
  ObjList<ObjT> loadInput(InputSource<InFmtT> const& list, ParseT const& parse) {
    static_assert(std::is_same<std::result_of(ParseT(InFmtT)), ObjT>::value);
  }
  
  template<typename ObjT, typename FuncT>
  void foreach(InputSource<ObjT> const& list, FuncT const& func) {
    
  }
    
  template<typename ObjT, typename FuncT>
  void foreach(DataFrame<ObjT> const& list, FuncT const& func) {
    
  }
  
  template<typename FromT, typename ToT, typename MapT>
  ObjList<ToT> map(DataFrame<FromT> const& list, MapT const& map) {
    static_assert(std::is_same<std::result_of(MapT(FromT,)), ToT>::value);
    
  }
  
  

}  // namespace execute
}  // namespace hydra
}  // namespace arangodb
