////////////////////////////////////////////////////////////////////////////////
/// @brief test suite for type casting in AQL
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2017 ArangoDB GmbH, Cologne, Germany
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
/// @author Copyright 2017, ArangoDB GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#include "catch.hpp"
#include "fakeit.hpp"

#include "Aql/Functions.h"
#include "Aql/Query.h"
#include "Aql/AqlValue.h"
#include "Basics/SmallVector.h"
#include "Transaction/Methods.h"

#include <velocypack/Builder.h>
#include <velocypack/Slice.h>
#include <velocypack/velocypack-aliases.h>

using namespace arangodb;
using namespace arangodb::aql;

namespace arangodb {
namespace tests {
namespace type_casting_aql {

SCENARIO("Testing if a Value IsBool", "[foobar]") {
  SmallVector<AqlValue>::allocator_type::arena_type arena;
  SmallVector<AqlValue> input{arena}; 
  fakeit::Mock<Query> queryMock;
  Query& query = queryMock.get();

  fakeit::Mock<transaction::Methods> trxMock;
  transaction::Methods& trx = trxMock.get();

  VPackBuilder tmpBuilder;

  THEN("IsBool of 'true' should be true") {
    tmpBuilder.add(VPackValue(true));
    input.emplace_back(tmpBuilder.slice());
    AqlValue res = Functions::IsBool(&query, &trx, input);
    REQUIRE(res.toBoolean() == true);
  }

  THEN("IsBool of 'false' should be true") {
    tmpBuilder.add(VPackValue(false));
    input.emplace_back(tmpBuilder.slice());
    AqlValue res = Functions::IsBool(&query, &trx, input);
    REQUIRE(res.toBoolean() == true);
  }

  THEN("IsBool of a number should be false") {
    tmpBuilder.add(VPackValue(2));
    input.emplace_back(tmpBuilder.slice());
    AqlValue res = Functions::IsBool(&query, &trx, input);
    REQUIRE(res.toBoolean() == false);
  }

  THEN("IsBool of a string should be false") {
    tmpBuilder.add(VPackValue("foobar"));
    input.emplace_back(tmpBuilder.slice());
    AqlValue res = Functions::IsBool(&query, &trx, input);
    REQUIRE(res.toBoolean() == false);
  }

  THEN("IsBool of an object should be false") {
    tmpBuilder.openObject();
    tmpBuilder.close();
    input.emplace_back(tmpBuilder.slice());
    AqlValue res = Functions::IsBool(&query, &trx, input);
    REQUIRE(res.toBoolean() == false);
  }

  THEN("IsBool of an array should be false") {
    tmpBuilder.openArray();
    tmpBuilder.close();
    input.emplace_back(tmpBuilder.slice());
    AqlValue res = Functions::IsBool(&query, &trx, input);
    REQUIRE(res.toBoolean() == false);
  }
}

}
}
}
