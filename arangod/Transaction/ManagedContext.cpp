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

#include "ManagedContext.h"
#include "StorageEngine/TransactionState.h"
#include "StorageEngine/TransactionManager.h"
#include "StorageEngine/TransactionManagerFeature.h"
#include "Utils/CollectionNameResolver.h"

struct TRI_vocbase_t;

namespace arangodb {
  
//static thread_local TRI_voc_tid_t CURRENT_TRX_ID;

/// @brief create the context
transaction::ManagedContext::ManagedContext(TRI_vocbase_t& vocbase,
                                            TRI_voc_tid_t tid,
                                            transaction::ManagedContext::Type ctxType)
  : Context(vocbase), _tid(tid), _ctxType(ctxType), _state(nullptr) {
  TRI_ASSERT(_tid != 0);
}
  
/// @brief create the context, will use given transaction
transaction::ManagedContext::ManagedContext(TRI_vocbase_t& vocbase, TransactionState* state)
  : Context(vocbase), _tid(state->id()), _ctxType(ManagedContext::Type::Internal), _state(state) {
    TRI_ASSERT(_state != nullptr);
    TRI_ASSERT(_state->isTopLevelTransaction());
}

/// @brief order a custom type handler for the collection
std::shared_ptr<arangodb::velocypack::CustomTypeHandler> transaction::ManagedContext::orderCustomTypeHandler() {
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
CollectionNameResolver const& transaction::ManagedContext::resolver() {
  if (_resolver == nullptr) {
    createResolver();
  }
  
  TRI_ASSERT(_resolver != nullptr);
  
  return *_resolver;
}
  
/// @brief get parent transaction (if any) and increase nesting
TransactionState* transaction::ManagedContext::leaseParentTransaction() {
  if (_state != nullptr) {
    // no single document transaction should have multiple Method instances
    TRI_ASSERT(_ctxType != ManagedContext::Type::Single);
    _state->increaseNesting();
    return _state;
  }
  if (_ctxType == ManagedContext::Type::Global) {
    TransactionManager* mgr = TransactionManagerFeature::manager();
    TRI_ASSERT(mgr != nullptr);
    // will call increaseNesting() for us
    _state = mgr->lookup(_tid, TransactionManager::Ownership::Lease);
    if (_state == nullptr) {
      THROW_ARANGO_EXCEPTION(TRI_ERROR_TRANSACTION_NOT_FOUND);
    }
    return _state;
  }
  TRI_ASSERT(_state == nullptr && _ctxType == Type::Single);
  return nullptr;
}

/// @brief register the transaction, so other Method instances can get it
void transaction::ManagedContext::registerTransaction(TransactionState* state) {
  TRI_ASSERT(_state == nullptr);
  TRI_ASSERT(_ctxType == Type::Single || state->hasHint(transaction::Hints::Hint::MANAGED));
  TRI_ASSERT(_tid == state->id());
  _state = state;
}
  
/// @brief unregister the transaction
void transaction::ManagedContext::unregisterTransaction() noexcept {
  _state = nullptr;
}
  
/// @brief whether or not the transaction is embeddable
bool transaction::ManagedContext::isEmbeddable() const {
  //TRI_ASSERT(CURRENT_TRX_ID != 0);
  return true;
}
  
TRI_voc_tid_t transaction::ManagedContext::generateId() const {
  if (_ctxType == Type::Global || _ctxType == Type::Internal) {
    THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_TRANSACTION_INTERNAL,
                                   "starting a new transaction is not allowed");
  }
  return _tid;
}
} // arangodb
