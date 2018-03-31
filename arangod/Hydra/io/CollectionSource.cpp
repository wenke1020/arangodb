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

#include "CollectionSource.h"
#include "Transaction/Methods.h"
#include "Transaction/StandaloneContext.h"
#include "Utils/SingleCollectionTransaction.h"
#include "Utils/OperationCursor.h"
#include "Utils/OperationResult.h"
#include "VocBase/vocbase.h"

using namespace arangodb;

hydra::CollectionSource::CollectionSource(transaction::Methods* trx,
                                          std::string const& cname) : _ownsTransaction(true), _trx(trx) {
  TRI_ASSERT(trx->status() == transaction::Status::RUNNING);
  if (trx->status() != transaction::Status::RUNNING) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_BAD_PARAMETER);
  }
  trx->addCollectionAtRuntime(cname);
  _cursor = trx->indexScan(cname, transaction::Methods::CursorType::ALL, false);
}

hydra::CollectionSource::CollectionSource(TRI_vocbase_t* vocbase,
                                          std::string const& cname) : _ownsTransaction(true), _trx(nullptr) {
  auto ctx = transaction::StandaloneContext::Create(vocbase);
  _trx = new SingleCollectionTransaction(ctx, cname, AccessMode::Type::READ);
  Result res = _trx->begin();
  if (res.fail()) {
    THROW_ARANGO_EXCEPTION(res);
  }
}

hydra::CollectionSource::~CollectionSource() {
  if (_ownsTransaction) {
    delete _trx;
  }
}


bool hydra::CollectionSource::hasNext() const {
  return _cursor->hasMore();
}

void hydra::CollectionSource::next(std::function<velocypack::Slice const& >const& func) {
  _cursor->nextDocument([&func](LocalDocumentId const& id, VPackSlice const& doc) {
    func(doc);
  }, 1000);
}

