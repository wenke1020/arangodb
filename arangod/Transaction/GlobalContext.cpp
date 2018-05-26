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
/// @author Jan Steemann
////////////////////////////////////////////////////////////////////////////////

#include "GlobalContext.h"
#include "StorageEngine/TransactionState.h"
#include "StorageEngine/TransactionManager.h"
#include "StorageEngine/TransactionManagerFeature.h"
#include "Utils/CollectionNameResolver.h"

struct TRI_vocbase_t;

namespace arangodb {
  
static thread_local TRI_voc_tid_t CURRENT_TRX_ID;

/// @brief create the context
transaction::GlobalContext::GlobalContext(TRI_vocbase_t& vocbase)
  : Context(vocbase) {
}

/// @brief order a custom type handler for the collection
std::shared_ptr<arangodb::velocypack::CustomTypeHandler> transaction::GlobalContext::orderCustomTypeHandler() {
  if (_customTypeHandler == nullptr) {
    _customTypeHandler.reset(
      transaction::Context::createCustomTypeHandler(&_vocbase, getResolver())
    );
    _options.customTypeHandler = _customTypeHandler.get();
    _dumpOptions.customTypeHandler = _customTypeHandler.get();
  }

  TRI_ASSERT(_customTypeHandler != nullptr);

  return _customTypeHandler;
}

/// @brief return the resolver
CollectionNameResolver const* transaction::GlobalContext::getResolver() {
  if (_resolver == nullptr) {
    createResolver();
  }
  TRI_ASSERT(_resolver != nullptr);
  return _resolver;
}
  
/// @brief return the parent transaction
TransactionState* transaction::GlobalContext::getParentTransaction() const {
  if (CURRENT_TRX_ID == 0) {
    return nullptr;
  }
  
  TransactionManager* mgr = TransactionManagerFeature::manager();
  TRI_ASSERT(mgr != nullptr);
  TransactionState* trx = mgr->lookup(CURRENT_TRX_ID);
  TRI_ASSERT(trx != nullptr);
  TRI_ASSERT(trx->hasHint(transaction::Hints::Hint::IS_GLOBAL));
  return trx;
}

/// @brief register the transaction,
void transaction::GlobalContext::registerTransaction(TransactionState* trx) {
  TRI_ASSERT(trx->hasHint(transaction::Hints::Hint::IS_GLOBAL));
  CURRENT_TRX_ID = trx->id();
}
  
/// @brief unregister the transaction
void transaction::GlobalContext::unregisterTransaction() noexcept {
  CURRENT_TRX_ID = 0;
}

/// @brief create a context, returned in a shared ptr
/*static*/ std::shared_ptr<transaction::GlobalContext> transaction::GlobalContext::Create(
    TRI_vocbase_t& vocbase
) {
  return std::make_shared<transaction::GlobalContext>(vocbase);
}

} // arangodb
