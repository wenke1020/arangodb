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

#ifndef ARANGOD_TRANSACTION_GLOBAL_CONTEXT_H
#define ARANGOD_TRANSACTION_GLOBAL_CONTEXT_H 1

#include "Context.h"
#include "Basics/Common.h"
#include "VocBase/vocbase.h"

struct TRI_vocbase_t;

namespace arangodb {

class TransactionState;

namespace transaction {

/// transaction context that will lease a TransactionState
/// from the TransactionManager
class LeasedContext final : public Context {
 public:

  /// @brief create the context, will try to lease transaction from manager
  explicit LeasedContext(TRI_vocbase_t& vocbase, TRI_voc_tid_t, bool allowCreating);
  /// @brief create the context, will use given transaction
  explicit LeasedContext(TRI_vocbase_t& vocbase, TransactionState*);

  /// @brief destroy the context
  ~LeasedContext() = default;

  /// @brief order a custom type handler
  std::shared_ptr<arangodb::velocypack::CustomTypeHandler>
  orderCustomTypeHandler() override final;

  /// @brief return the resolver
  CollectionNameResolver const& resolver() override final;

  /// @brief get parent transaction (if any) and increase nesting
  TransactionState* leaseParentTransaction() const override;

  /// @brief register the transaction,
  void registerTransaction(TransactionState*) override;

  /// @brief unregister the transaction
  void unregisterTransaction() noexcept override;

  /// @brief whether or not the transaction is embeddable
  bool isEmbeddable() const override;
  
  /// @brief generate a transaction ID for this
  TRI_voc_tid_t generateId() const override;
  
private:
  /// @brief ID of the transaction to use
  TRI_voc_tid_t const _tid;
  bool const _allowCreatingNew;
  TransactionState *_state;
};

}
}

#endif
