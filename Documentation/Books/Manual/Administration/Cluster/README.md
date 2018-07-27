Cluster Administration
======================

This _Section_ includes information related to the administration of an ArangoDB Cluster.

For a general introduction to the ArangoDB Cluster, please refer to the
Cluster [chapter](../../Scalability/Cluster/README.md).

Please also check the following talks:

| # | Date            | Title                                                                       | Who                                     | Link                                                                                                            |
|---|-----------------|-----------------------------------------------------------------------------|-----------------------------------------|----------------------------------------------------------------------------------------------------------------------------------------------|
| 1 | 10th April 2018 | Fundamentals and Best Practices of ArangoDB Cluster Administration          | Kaveh Vahedipour, ArangoDB Cluster Team | [Online Meetup Page](https://www.meetup.com/online-ArangoDB-meetup/events/248996022/) & [Video](https://www.youtube.com/watch?v=RQ33fkgUg64) |
| 2 | 29th May 2018   | Fundamentals and Best Practices of ArangoDB Cluster Administration: Part II | Kaveh Vahedipour, ArangoDB Cluster Team | [Online Meetup Page](https://www.meetup.com/online-ArangoDB-meetup/events/250869684/) & [Video](https://www.youtube.com/watch?v=jj7YpTaL3pI) |


Enabling synchronous replication
--------------------------------

For an introduction about _Synchronous Replication_ in Cluster, please refer
to the [_Cluster Architecture_](../../Scalability/Cluster/Architecture.md#synchronous-replication) section. 
                                                               
Synchronous replication can be enabled per _collection_. When creating a
_collection_ you may specify the number of _replicas_ using the
*replicationFactor* parameter. The default value is set to `1` which
effectively *disables* synchronous replication among _DBServers_. 

Whenever you specify a _replicationFactor_ greater than 1 when creating a
collection, synchronous replication will be activated for this collection. 
The Cluster will determine suitable _leaders_ and _followers_ for every 
requested _shard_ (_numberOfShards_) within the Cluster.

Example:

```
127.0.0.1:8530@_system> db._create("test", {"replicationFactor": 3})
```

In the above case, any write operation will require 2 replicas to
report success from now on. 

Preparing growth
----------------

You may create a _collection_ with higher _replication factor_ than
available _DBServers_. When additional _DBServers_ become available 
the _shards_ are automatically replicated to the newly available _DBServers_. 

To create a _collection_ with higher _replication factor_ than
available _DBServers_ please set the option _enforceReplicationFactor_ to _false_, 
when creating the collection from _ArangoShell_ (the option is not available
from the web interface), e.g.:

```
db._create("test", { replicationFactor: 4 }, { enforceReplicationFactor: false });
```

The default value for _enforceReplicationFactor_ is true. 

**Note:** multiple _replicas_ of the same _shard_ can never coexist on the same
_DBServer_ instance.

Sharding
--------

For an introduction about _Sharding_ in Cluster, please refer to the
[_Cluster Architecture_](../../Scalability/Cluster/Architecture.md#sharding) section. 

Number of _shards_ can be configured at _collection_ creation time, e.g. the UI,
or the _ArangoDB Shell_:

```
127.0.0.1:8529@_system> db._create("sharded_collection", {"numberOfShards": 4});
```

To configure a custom _hashing_ for another attribute (default is __key_):

```
127.0.0.1:8529@_system> db._create("sharded_collection", {"numberOfShards": 4, "shardKeys": ["country"]});
```

The example above, where 'country' has been used as _shardKeys_ can be useful
to keep data of every country in one shard, which would result in better
performance for queries working on a per country base. 

It is also possible to specify multiple `shardKeys`. 

Note however that if you change the shard keys from their default `["_key"]`, then finding
a document in the collection by its primary key involves a request to
every single shard. Furthermore, in this case one can no longer prescribe
the primary key value of a new document but must use the automatically
generated one. This latter restriction comes from the fact that ensuring
uniqueness of the primary key would be very inefficient if the user
could specify the primary key.

On which DBServer in a Cluster a particular _shard_ is kept is undefined.
There is no option to configure an affinity based on certain _shard_ keys.

Unique indexes (hash, skiplist, persistent) on sharded collections are
only allowed if the fields used to determine the shard key are also
included in the list of attribute paths for the index:

| shardKeys | indexKeys |             |
|----------:|----------:|------------:|
| a         | a         |     allowed |
| a         | b         | not allowed |
| a         | a, b      |     allowed |
| a, b      | a         | not allowed |
| a, b      | b         | not allowed |
| a, b      | a, b      |     allowed |
| a, b      | a, b, c   |     allowed |
| a, b, c   | a, b      | not allowed |
| a, b, c   | a, b, c   |     allowed |

Moving/Rebalancing _shards_
---------------------------

A _shard_ can be moved from a _DBServer_ to another, and the entire shard distribution
can be rebalanced using the correponding buttons in the web [UI](../../Programs/WebInterface/Cluster.md).

## Replacing Agents

At times it will become necessary to replace one, multiple or all
agents. Before getting into the details, one needs to realize that
replacing agents should under all circumstances happen one at a
time. The reasons for which are in detail rather complicated and
lengthy and well outside of the scope of this document. Consequently,
RAFT dictates the circumstances which have to pertain for replacements
to be viable.

As it is of utmost importance that the agency is not corrupted, it
cannot be stressed enough how important it is to replace an agent at a time.

Let us start by understanding how the agency's current situation is
acquired.

```
curl <agent-x-ip:agent-x-port>/_api/agency/config
```

Will output something like below:

```
{
	  "term": 1,
	  "leaderId": "AGNT-68545574-183a-46de-8817-291d425ae381",
	  "commitIndex": 1203,
	  "lastAcked": {},
	  "configuration": {
	    "pool": {
	      "AGNT-240cd261-b6a9-4f79-afb6-57c95c39f42b": "tcp://c1:5000",
	      "AGNT-bbb658a2-7874-4392-9c38-3be3b74a76ea": "tcp://c2:5000",
	      "AGNT-68545574-183a-46de-8817-291d425ae381": "tcp://c3:5000"
	    },
	    "active": [
	      "AGNT-240cd261-b6a9-4f79-afb6-57c95c39f42b",
	      "AGNT-bbb658a2-7874-4392-9c38-3be3b74a76ea",
	      "AGNT-68545574-183a-46de-8817-291d425ae381"
	    ],
	    "id": "AGNT-bbb658a2-7874-4392-9c38-3be3b74a76ea",
	    "agency size": 3,
	    "pool size": 3,
	    "endpoint": "tcp://c1:5000",
	    "min ping": 1,
	    "max ping": 5,
	    "timeoutMult": 1,
	    "supervision": true,
	    "supervision frequency": 1,
	    "compaction step size": 2000,
	    "compaction keep size": 1000,
	    "supervision grace period": 5,
	    "version": 3,
	    "startup": "origin"
	  }
	}
```
The above is to be decyphered as follows:

This agency started to do RAFT in,`term: 1`. It consists of 3
agents,`agency size: 3`. This agent is currently a follower. This is
visible both in comparing `id` and `leaderId`, which are different and
a more subtle place to note is the empty object at`lastAcked`.

The RAFT upper and lower timeouts are at 1 and 5 seconds. The
supervision is activated. The replicated log is compatacted every 2000
entries, while keeping a backlog of 1000 entries. After 5 seconds of
missing out heartbeats from db servers action is taken.

When querying the configuration of the leader at the same time,

```
"leaderId": "AGNT-68545574-183a-46de-8817-291d425ae381"
```
whose endpoint is `tcp://c3:5000` we will see the following output:

```
{
	  "term": 1,
	  "leaderId": "AGNT-68545574-183a-46de-8817-291d425ae381",
	  "commitIndex": 1204,
	  "lastAcked": {
	    "AGNT-68545574-183a-46de-8817-291d425ae381": 0,
	    "AGNT-240cd261-b6a9-4f79-afb6-57c95c39f42b": 0.16,
	    "AGNT-bbb658a2-7874-4392-9c38-3be3b74a76ea": 0.16
	  },
	  "configuration": {
	    "pool": {
	      "AGNT-240cd261-b6a9-4f79-afb6-57c95c39f42b": "tcp://c1:5000",
	      "AGNT-68545574-183a-46de-8817-291d425ae381": "tcp://c3:5000",
	      "AGNT-bbb658a2-7874-4392-9c38-3be3b74a76ea": "tcp://c2:5000"
	    },
	    "active": [
	      "AGNT-240cd261-b6a9-4f79-afb6-57c95c39f42b",
	      "AGNT-68545574-183a-46de-8817-291d425ae381",
	      "AGNT-bbb658a2-7874-4392-9c38-3be3b74a76ea"
	    ],
	    "id": "AGNT-68545574-183a-46de-8817-291d425ae381",
	    "agency size": 3,
	    "pool size": 3,
	    "endpoint": "tcp://c3:5000",
	    "min ping": 1,
	    "max ping": 5,
	    "timeoutMult": 1,
	    "supervision": true,
	    "supervision frequency": 1,
	    "compaction step size": 2000,
	    "compaction keep size": 1000,
	    "supervision grace period": 5,
	    "version": 3,
	    "startup": "origin"
	  }
	}
```

Note that `lastAcked` now display the last time, that communication
between this agent and its follower was acknowledged, which is
naturally 0 for the leader itself and 0.16s for either one
follower. Also there is a match between `leader` and
`configuration/id`.

A situation, where one of the followers is down is visible in checking
back with the `lastAcked` object on the leader like so:

```
"lastAcked": {
	  "AGNT-68545574-183a-46de-8817-291d425ae381": 0,
	  "AGNT-240cd261-b6a9-4f79-afb6-57c95c39f42b": 0.07,
	  "AGNT-bbb658a2-7874-4392-9c38-3be3b74a76ea": 1109.32
	}
```	

Here, the agent at endpoint `tcp://c2:5000` hast gone for 1109 seconds
for almost 18 and half minutes. This is a situation, that should not
be tolerated very much comparable of a dead hard disk in a RAID 3
set. It needs immediate replacement.

### Replacing an agent with its data intact

The first duty in any replacement measure is backing up! Backup whatever you have. This point cannot be stressed enough. 1000s of stories ranging from fun und absolute desaster are told over beer and pickles that start with. I knew, what I was doing, but ...

If the data directory of the missing agent is still available, one may start an `arangod` with the same parameters as the other two agents while including at least one of the other 2 agents with `--agency.endpoint tcp://c1:5000` `--agency.endpoint tcp://c3:5000` `--agency.my-address tcp://c2:5000.` The first two parameters are meant to announce the other agents' endpoints while the last advertises the "public" ip address under which the other agents may contact this newly started agent process.

### Replacing an agent in disaster recovery mode (3.2.7 and above)

If the data directory of the failed agent is no longer available one may replace an agent with if and oly if the majorty of the entire agency is still operational and RAFTing. By starting with a clean database directory while additionally specfying `--agency.disaster-recovery-id <agent-id>` in the above case the agent, which has not been responding anymore for over 18 minutes. In that case, the reborn agent is brought up to speed including configuration and the replicated log by the RAFT leader.

At all costs one must make sure that agent ids are not mistaken.

Replacing/Removing a _Coordinator_
----------------------------------

_Coordinators_ are effectively stateless and can be replaced, added and
removed without more consideration than meeting the necessities of the
particular installation. 

To take out a _Coordinator_ stop the
_Coordinator_'s instance by issueing `kill -SIGTERM <pid>`.

Ca. 15 seconds later the cluster UI on any other _Coordinator_ will mark
the _Coordinator_ in question as failed. Almost simultaneously, a trash bin
icon will appear to the right of the name of the _Coordinator_. Clicking
that icon will remove the _Coordinator_ from the coordinator registry.

Any new _Coordinator_ instance that is informed of where to find any/all
agent/s, `--cluster.agency-endpoint` `<some agent endpoint>` will be
integrated as a new _Coordinator_ into the cluster. You may also just
restart the _Coordinator_ as before and it will reintegrate itself into
the cluster.

Replacing/Removing a _DBServer_
-------------------------------

_DBServers_ are where the data of an ArangoDB cluster is stored. They
do not publish a we UI and are not meant to be accessed by any other
entity than _Coordinators_ to perform client requests or other _DBServers_
to uphold replication and resilience.

The clean way of removing a _DBServer_ is to first releave it of all
its responsibilities for shards. This applies to _followers_ as well as
_leaders_ of shards. The requirement for this operation is that no
collection in any of the databases has a `relicationFactor` greater or
equal to the current number of _DBServers_ minus one. For the pupose of
cleaning out `DBServer004` for example would work as follows, when
issued to any _Coordinator_ of the cluster:

`curl <coord-ip:coord-port>/_admin/cluster/cleanOutServer -d '{"id":"DBServer004"}'`

After the _DBServer_ has been cleaned out, you will find a trash bin
icon to the right of the name of the _DBServer_ on any _Coordinators_'
UI. Clicking on it will remove the _DBServer_ in questiuon from the
cluster.

Firing up any _DBServer_ from a clean data directory by specifying the
any of all agency endpoints will integrate the new _DBServer_ into the
cluster.

To distribute shards onto the new _DBServer_ either click on the
`Distribute Shards` button at the bottom of the `Shards` page in every
database.

