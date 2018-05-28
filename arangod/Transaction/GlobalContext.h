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

class GlobalContext final : public Context {
 public:

  /// @brief create the context
  explicit GlobalContext(TRI_vocbase_t& vocbase);

  /// @brief destroy the context
  ~GlobalContext() = default;

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

  /// @brief create a context, returned in a shared ptr
  static std::shared_ptr<transaction::GlobalContext> Create(
    TRI_vocbase_t& vocbase);
  
  /// @brief register as the current transaction
  static void registerTransaction(TRI_voc_tid_t tid);
};

}
}

#endif
