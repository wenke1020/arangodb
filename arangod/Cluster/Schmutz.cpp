#include "Schmutz.h"
#include "Cluster/ServerState.h"

using namespace arangodb;

void Schmutz::synchronizeOneShard (
  std::string const& database, std::string const& shard, size_t planId,
  std::string const& leader) {

  std::string ourselves = ServerState::instance()->getId();
  
  
}

