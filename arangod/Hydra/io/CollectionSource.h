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

#ifndef ARANGODB_HYDRA_COLLECTION_SOURCE_H
#define ARANGODB_HYDRA_COLLECTION_SOURCE_H 1

#include "Hydra/io/InputSource.h"
#include <string>
#include <velocypack/Slice.h>
#include "Utils/OperationCursor.h"

struct TRI_vocbase_t;
namespace arangodb {
namespace transaction {
  class Methods;
}
  
namespace hydra {
  class CollectionSource : public InputSource<velocypack::Slice const&> {
  public:
    CollectionSource(transaction::Methods*, std::string const& cname);
    CollectionSource(TRI_vocbase_t* vocbase, std::string const& cname);
    ~CollectionSource();
    
    bool hasMore() const override {
      return _cursor->hasMore();
    }
    
    bool next(std::function<void(velocypack::Slice const&)> const& func, size_t batch) override {
      return _cursor->nextDocument([&func](LocalDocumentId const& id, VPackSlice const& doc) {
        func(doc);
      }, batch);
    }

  private:
    bool _ownsTransaction;
    transaction::Methods* _trx;
    std::unique_ptr<OperationCursor> _cursor;
  };
}
}
#endif
