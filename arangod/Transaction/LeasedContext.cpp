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

#include "LeasedContext.h"
#include "StorageEngine/TransactionState.h"
#include "StorageEngine/TransactionManager.h"
#include "StorageEngine/TransactionManagerFeature.h"
#include "Utils/CollectionNameResolver.h"

struct TRI_vocbase_t;

namespace arangodb {
  
//static thread_local TRI_voc_tid_t CURRENT_TRX_ID;

/// @brief create the context
transaction::LeasedContext::LeasedContext(TRI_vocbase_t& vocbase,
                                          TRI_voc_tid_t tid,
                                          bool allowCreating)
  : Context(vocbase), _tid(tid), _allowCreatingNew(allowCreating), _state(nullptr) {
  TRI_ASSERT(_tid != 0);
}
  
/// @brief create the context, will use given transaction
transaction::LeasedContext::LeasedContext(TRI_vocbase_t& vocbase, TransactionState* state)
  : Context(vocbase), _tid(state->id()), _allowCreatingNew(false), _state(state) {
    TRI_ASSERT(_state != nullptr);
    TRI_ASSERT(_state->isTopLevelTransaction());
}

/// @brief order a custom type handler for the collection
std::shared_ptr<arangodb::velocypack::CustomTypeHandler> transaction::LeasedContext::orderCustomTypeHandler() {
  if (_customTypeHandler == nullptr) {
    _customTypeHandler.reset(
      transaction::Context::createCustomTypeHandler(&_vocbase, &resolver())
    );
    _options.customTypeHandler = _customTypeHandler.get();
    _dumpOptions.customTypeHandler = _customTypeHandler.get();
  }

  TRI_ASSERT(_customTypeHandler != nullptr);

  return _customTypeHandler;
}

/// @brief return the resolver
CollectionNameResolver const& transaction::LeasedContext::resolver() {
  if (_resolver == nullptr) {
    createResolver();
  }
  
  TRI_ASSERT(_resolver != nullptr);
  
  return *_resolver;
}
  
/// @brief get parent transaction (if any) and increase nesting
TransactionState* transaction::LeasedContext::leaseParentTransaction() const {
  if (_state != nullptr) {
    _state->increaseNesting();
    return _state;
  }
  TransactionManager* mgr = TransactionManagerFeature::manager();
  TRI_ASSERT(mgr != nullptr);
  // will call increaseNesting() for us
  TransactionState* state = mgr->lookup(_tid, TransactionManager::Ownership::Lease);
  // FIXME: fix these asserts
  TRI_ASSERT(_allowCreatingNew || state != nullptr);
  TRI_ASSERT(state == nullptr || state->hasHint(transaction::Hints::Hint::EL_CHEAPO));
  return state;
}

/// @brief register the transaction, so other Method instances can get it
void transaction::LeasedContext::registerTransaction(TransactionState* state) {
  TRI_ASSERT(_allowCreatingNew || state->hasHint(transaction::Hints::Hint::EL_CHEAPO));
  //_tid = trx->id();
  TRI_ASSERT(_tid == state->id());
  _state = state; // does not necessarily need to happen here
}
  
/// @brief unregister the transaction
void transaction::LeasedContext::unregisterTransaction() noexcept {
  //_tid = 0;
  _state = nullptr;
}
  
/// @brief whether or not the transaction is embeddable
bool transaction::LeasedContext::isEmbeddable() const {
  //TRI_ASSERT(CURRENT_TRX_ID != 0);
  return true;
}
  
TRI_voc_tid_t transaction::LeasedContext::generateId() const {
  if (!_allowCreatingNew) {
    THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_TRANSACTION_INTERNAL,
                                   "starting a new transaction is not allowed");
  }
  return _tid;
}

} // arangodb
