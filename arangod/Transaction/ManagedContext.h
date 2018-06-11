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

/// transaction context that will manage the creation or aquisition of a TransactionState
/// for transction::Methods instances for cluster wide transactions. Cluster wide transactions
/// essentially just mean that all operations will use a consistent transaction ID and
/// on the same server the same TransactionState instance will be used.
/// The class supports three different use-cases
/// (1) Constructor with TID and Type::Default can be used to share a TransactionState between
///     multiple transaction::Methods instances
/// (2) Constructor with TID and Type::Global will try to lease an already existing TransactionState
///     from the TransactionManager. This supports global transaction with explicit begin / end requests
/// (3) Construcor with TransactionState* is used to manage a global transaction
class ManagedContext final : public Context {
 public:

  enum class Type {
    Default = 0, /// transaction with pre-defined ID
    Global = 1, /// global transaction with begin / end semantics
    Internal = 2
  };
  
  /// @brief create the context, with given TID
  explicit ManagedContext(TRI_vocbase_t& vocbase, TRI_voc_tid_t, Type ctxType);
  /// @brief create the context, will use given TransactionState
  explicit ManagedContext(TRI_vocbase_t& vocbase, TransactionState*);

  /// @brief destroy the context
  ~ManagedContext() = default;

  /// @brief order a custom type handler
  std::shared_ptr<arangodb::velocypack::CustomTypeHandler>
  orderCustomTypeHandler() override final;

  /// @brief return the resolver
  CollectionNameResolver const& resolver() override final;

  /// @brief get parent transaction (if any) and increase nesting
  TransactionState* leaseParentTransaction() override;

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
  /// @brief is this managing a global context
  ManagedContext::Type const _ctxType;
  /// @brief managed TransactionState
  TransactionState *_state;
};

}
}

#endif
