////////////////////////////////////////////////////////////////////////////////
/// @brief test suite for Aql Array functions
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
#include "Transaction/Context.h"
#include "Transaction/Methods.h"

#include <velocypack/Builder.h>
#include <velocypack/Slice.h>
#include <velocypack/velocypack-aliases.h>

using namespace arangodb;
using namespace arangodb::aql;

namespace arangodb {
namespace tests {
namespace test_aql_functions {

SCENARIO("Testing NOT_NULL aql function", "[aql][aqlfunctions]") {
  // everything here

  fakeit::Mock<Query> queryMock;
  Query& query = queryMock.get();

  fakeit::Mock<transaction::Methods> trxMock;
  transaction::Methods& trx = trxMock.get();

  SmallVector<AqlValue>::allocator_type::arena_type arena;
  SmallVector<AqlValue> inputParameters{arena};

  GIVEN("an empty list") {
    AqlValue res = Functions::NotNull(&query, &trx, inputParameters);

    THEN("The result should be null") {
      REQUIRE(res.isNull(false) == true);
    }
    // everything here
  }
  GIVEN("a list with a non null element") {
    inputParameters.emplace_back(AqlValueHintUInt(3));
    AqlValue res = Functions::NotNull(&query, &trx, inputParameters);
    // not known here
    THEN("The result should not be null") {
      REQUIRE(res.isNull(false) == false);
    }

    THEN("The result should be 3") {
      VPackSlice slice = res.slice();
      uint64_t nrResult = slice.getNumber<uint64_t>();
      REQUIRE(nrResult == 3);
    }
  }
}

SCENARIO("Testing CURRENT_DATABASE", "[aql][aqlfunctions]") {

  std::string dbName = "UnitTestDB";
  fakeit::Mock<Query> queryMock;
  Query& query = queryMock.get();

  fakeit::Mock<transaction::Methods> trxMock;
  transaction::Methods& trx = trxMock.get();

  fakeit::Mock<transaction::Context> ctxtMock;
  transaction::Context& ctxt = ctxtMock.get();

  fakeit::Mock<TRI_vocbase_t> vocbaseMock;
  TRI_vocbase_t& vocbase = vocbaseMock.get();

  VPackBuilder leasedBuilder;

  SmallVector<AqlValue>::allocator_type::arena_type arena;
  SmallVector<AqlValue> inputParameters{arena};

  // fakeit::When(Method(trxMock, transactionContextPtr)).AlwaysReturn(&ctxt);
  fakeit::When(Method(ctxtMock, leaseBuilder)).AlwaysReturn(&leasedBuilder);
  fakeit::When(Method(ctxtMock, returnBuilder)).AlwaysReturn();
  fakeit::When(Method(queryMock, vocbase)).AlwaysReturn(&vocbase);
  fakeit::When(Method(vocbaseMock, name)).AlwaysReturn(dbName);


  GIVEN("Empty input") {

    THEN("We should get the real vocbase") {
      AqlValue res = Functions::CurrentDatabase(&query, &trx, inputParameters);
      REQUIRE(!res.isNull(false));
    }
  }

}

}
}
}
