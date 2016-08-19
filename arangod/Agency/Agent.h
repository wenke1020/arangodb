////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2014-2016 ArangoDB GmbH, Cologne, Germany
/// Copyright 2004-2014 triAGENS GmbH, Cologne, Germany
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
/// @author Kaveh Vahedipour
////////////////////////////////////////////////////////////////////////////////

#ifndef ARANGOD_CONSENSUS_AGENT_H
#define ARANGOD_CONSENSUS_AGENT_H 1

#include "Agency/AgencyCommon.h"
#include "Agency/AgentConfiguration.h"
#include "Agency/AgentCallback.h"
#include "Agency/Constituent.h"
#include "Agency/Supervision.h"
#include "Agency/State.h"
#include "Agency/Store.h"
#include "Agency/Inception.h"

struct TRI_server_t;
struct TRI_vocbase_t;

namespace arangodb {
namespace consensus {
class Agent : public arangodb::Thread {
 public:
  /// @brief Construct with program options
  explicit Agent(config_t const&);

  /// @brief Clean up
  ~Agent();

  /// @brief Get current term
  term_t term() const;

  /// @brief Get current term
  std::string id() const;

  /// @brief Vote request
  priv_rpc_ret_t requestVote(term_t, std::string const&, index_t,
                             index_t, query_t const&);

  /// @brief Provide configuration
  config_t const config() const;

  /// @brief Start thread
  bool start();

  /// @brief My endpoint
  std::string endpoint() const;

  /// @brief Verbose print of myself
  void print(arangodb::LoggerStream&) const;

  /// @brief Are we fit to run?
  bool fitness() const;

  /// @brief Leader ID
  arangodb::consensus::index_t lastCommitted() const;

  /// @brief Leader ID
  std::string leaderID() const;

  /// @brief Are we leading?
  bool leading() const;

  /// @brief Pick up leadership tasks
  bool lead();

  /// @brief Load persistent state
  bool load();

  /// @brief Attempt write
  write_ret_t write(query_t const&);

  /// @brief Read from agency
  read_ret_t read(query_t const&);

  /// @brief Received by followers to replicate log entries ($5.3);
  ///        also used as heartbeat ($5.2).
  bool recvAppendEntriesRPC(
    term_t term, std::string const& leaderId, index_t prevIndex,
    term_t prevTerm, index_t lastCommitIndex, query_t const& queries);

  /// @brief Invoked by leader to replicate log entries ($5.3);
  ///        also used as heartbeat ($5.2).
  priv_rpc_ret_t sendAppendEntriesRPC(std::string const& slave_id);

  /// @brief 1. Deal with appendEntries to slaves.
  ///        2. Report success of write processes.
  void run() override final;

  /// @brief Are we still booting?
  bool booting();

  /// @brief Gossip in
  query_t gossip(query_t const&, bool callback = false);

  /// @brief Persisted agents
  bool persistedAgents();
  
  /// @brief Gossip in
  bool activeAgency();
  
  /// @brief Startup process of detection of agent pool, active agency, gossip etc
//  void inception();

  /// @brief Start orderly shutdown of threads
  void beginShutdown() override final;

  /// @brief Report appended entries from AgentCallback
  void reportIn(std::string const& id, index_t idx);

  /// @brief Wait for slaves to confirm appended entries
  bool waitFor(index_t last_entry, double timeout = 2.0);

  /// @brief Convencience size of agency
  size_t size() const;

  /// @brief Rebuild DBs by applying state log to empty DB
  bool rebuildDBs();

  /// @brief Last log entry
  log_t const& lastLog() const;

  /// @brief State machine
  State const& state() const;

  /// @brief Get read store
  Store const& readDB() const;

  /// @brief Get spearhead store
  Store const& spearhead() const;

  /// @brief Serve active agent interface
  bool serveActiveAgent();

  void startConstituent();
  
  /// State reads persisted state and prepares the agent
  friend class State;

 private:

  /// @brief Activate this agent in single agent mode.
  bool activateAgency();

  /// @brief Assignment of persisted state
  Agent& operator=(VPackSlice const&);

  /// @brief Get current term
  bool id(std::string const&);

  /// @brief Get current term
  bool mergeConfiguration(VPackSlice const&);

  /// @brief Leader ID
  void lastCommitted(arangodb::consensus::index_t);

  /// @brief This server (need endpoint)
  TRI_server_t* _server;

  /// @brief Vocbase for agency persistence
  TRI_vocbase_t* _vocbase;

  /// @brief Query registry for agency persistence
  aql::QueryRegistry* _queryRegistry;

  /// @brief Leader election delegate
  Constituent _constituent;

  /// @brief Cluster supervision module
  Supervision _supervision;

  /// @brief State machine
  State _state;

  /// @brief Configuration of command line options
  config_t _config;

  /// @brief Last commit index (raft)
  index_t _lastCommitIndex;

  /// @brief Spearhead (write) kv-store
  Store _spearhead;

  /// @brief Committed (read) kv-store
  Store _readDB;

  /// @brief Condition variable for appendEntries
  arangodb::basics::ConditionVariable _appendCV;

  /// @brief Condition variable for waitFor
  arangodb::basics::ConditionVariable _waitForCV;

  /// @brief Condition variable for waitFor
  arangodb::basics::ConditionVariable _configCV;

  /// @brief Confirmed indices of all members of agency
  //std::vector<index_t> _confirmed;
  std::map<std::string, index_t> _confirmed;
  std::map<std::string, index_t> _lastHighest;

  std::map<std::string, TimePoint> _lastSent;
  arangodb::Mutex _ioLock; /**< @brief Read/Write lock */

  /// @brief Server active agents rest handler
  bool _serveActiveAgent;

  /// @brief Next compaction after
  arangodb::consensus::index_t _nextCompationAfter;

  std::map<std::string, bool> _gossipTmp;

  std::unique_ptr<Inception> _inception;


};

}
}

#endif
