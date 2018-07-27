Using the ArangoDB Starter
==========================

This section describes how to start a Cluster using the tool [_Starter_](../../Programs/Starter/README.md)
(the _arangodb_ binary program).

Local Tests
-----------

If you only want a local test Cluster, you can run a single _Starter_ with the 
`--starter.local` argument. It will start a 3 "machine" Cluster on your local PC:

```
arangodb --starter.local --starter.data-dir=./localdata
```

**Note:** a local Cluster is intended only for test purposes since a failure of 
a single PC will bring down the entire Cluster.

Multiple Machines
-----------------

If you want to start a Cluster using the _Starter_, you can use the following command:

```
arangodb --starter.data-dir=./data --starter.join A,B,C
```
By invoking the first `arangodb` you launch a primary node. It will bind a network port, and output the commands you need to cut'n'paste into the other nodes.

Run the above command on machine A, B & C. 

Once all the processes started by the _Starter_ are up and running, and joined the
Cluster (this may take a while depending on your system), the _Starter_ will inform
you where to connect the Cluster from a Browser, shell or your program.
At this point you may access your cluster at either coordinator endpoint. 

Please note that without specifying the storage engine, your _Cluster_ will use _MMFiles_ as default storage engine. We suggest to use _RocksDB_ as storage engine, this can achieved by adding the following option to the _Starter_ command line:

```
--server.storage-engine rocksdb
```
For a comparison between _MMFiles_ and _RocksDB_ please refer to this [page](https://www.arangodb.com/why-arangodb/comparing-rocksdb-mmfiles-storage-engines/)

For a full list of options of the _Starter_ please refer to [this](../../Programs/Starter/Options.md)
section.

Using the ArangoDB Starter in Docker
------------------------------------

The _Starter_ can also be used to launch Clusters based on _Docker_ containers:

In the Docker world you need to take care about where persistent data is stored, since containers are intended to be volatile. We use a volume named `arangodb1` here:

```
docker volume create arangodb1
```
(You can use any type of docker volume that fits your setup instead.)

We then need to determine the IP of the docker host where you intend to run ArangoDB starter on. Depending on your operating system execute ip addr, ifconfig or ipconfig to determine your local ip address.

So this example uses the IP `172.17.0.12`:

```
docker run -it --name=adb1 --rm -p 8528:8528 \
    -v arangodb1:/data \
    -v /var/run/docker.sock:/var/run/docker.sock \
    arangodb/arangodb-starter \
    --starter.address=172.17.0.1 \
	--starter.join=A,B,C
```
It will start the master instance, and command you to start the slave instances:

```
Unable to find image 'arangodb/arangodb-starter:latest' locally
latest: Pulling from arangodb/arangodb-starter
81033e7c1d6a: Pull complete 
a75acbb9ae03: Pull complete 
Digest: sha256:65459917d300acd1cadb2936abe0f865250fa2ef8dace82428b0b5d2c8988870
Status: Downloaded newer image for arangodb/arangodb-starter:latest
2018-07-27T08:27:04Z |INFO| Starting arangodb version 0.12.0, build cd81a60 component=arangodb
2018-07-27T08:27:04Z |INFO| Serving as master with ID '6523b341' on 172.17.0.1:8528... component=arangodb
2018-07-27T08:27:04Z |INFO| Waiting for 3 servers to show up.
 component=arangodb
2018-07-27T08:27:04Z |INFO| Use the following commands to start other servers: component=arangodb

docker volume create arangodb2 && \
    docker run -it --name=adb2 --rm -p 8538:8528 -v arangodb2:/data \
    -v /var/run/docker.sock:/var/run/docker.sock arangodb/arangodb-starter:latest \
    --starter.address=172.17.0.1 --starter.join=172.17.0.1

docker volume create arangodb3 && \
    docker run -it --name=adb3 --rm -p 8548:8528 -v arangodb3:/data \
    -v /var/run/docker.sock:/var/run/docker.sock arangodb/arangodb-starter:latest \
    --starter.address=172.17.0.1 --starter.join=172.17.0.1
```
Run the above command on machines A, B & C.

Once you start the other instances, it will continue like this:

```
Added master '6523b341': 172.17.0.1, portOffset: 0
Added new peer 'e98ea757': 172.17.0.1, portOffset: 5
Added new peer 'eb01d0ef': 172.17.0.1, portOffset: 10
Starting service...
Looking for a running instance of agent on port 8531
Starting agent on port 8531
Looking for a running instance of dbserver on port 8530
Starting dbserver on port 8530
Looking for a running instance of coordinator on port 8529
Starting coordinator on port 8529
agent up and running (version 3.3.12).
dbserver up and running (version 3.3.12).
coordinator up and running (version 3.3.12).
```

And at least it tells you where you can work with your cluster:

```
Your cluster can now be accessed with a browser at `http://172.17.0.1:8529` or
using `arangosh --server.endpoint tcp://172.17.0.1:8529`.
```
Under the Hood
--------------
The first `arangodb` you ran will become the _master_ of your _Starter_
setup, the other `arangodb` instances will become the _slaves_ of your _Starter_
setup. Please do not confuse the terms _master_ and _slave_ above with the master/slave
technology of ArangoDB. The terms above refers to the _Starter_ setup.

The _Starter_ _master_ determines which ArangoDB server processes to launch on which
_Starter_ _slave_, and how they should communicate. 

It will then launch the server processes and monitor them. Once it has detected
that the setup is complete you will get the prompt. 

The _Starter_ _master_ will save the setup for subsequent starts. 

### A few notes on the starter's supervisor role

When an ArangoDB cluster is built using the starter, every process,
which is gone will automatically be respawned. This effectively means,
that if you want to test ArangoDB's fault resistance and resilience,
you will have to kill the `arangodb` process, before you can
gracefully shutdown or brutally kill a particular process in order to
not have a new process respawned in its place. And by killing this
document refers to `kill -SIGKILL`. If one uses `kill -SIGTERM`, on
the starter, all processes, which were started through that starter
instance will be shutdown.


