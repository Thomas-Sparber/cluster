# cluster
The cluster-database is a relational database is a relational database that stores data across many nodes. The background for the database is a peer-2-peer network which is responsible for searching other clients and exchanging information between them. This peer-2-peer network makes it possible to keep the configuration effort at a minimum (or even zero) level

The cluster-database combines the advantages of several technologies into a single product.
It is also designed in a flexible way which allows to attach or detach several modules.
Those modules consist of:
 - Network layer: The network layer is the layer that is responsible for the communication between the clients.
   Currently supported protocols are IPv4 and IPv6 but it is easy to implement e.g. SSL over IPv4/6 or even a proprietary communication technology.
 - Customer-objects: Cluster objects are objects which operate in the clustered environment. Currently implemented cluster-objects are:
    - P2P-network: The peer-2-peer network is the basis for the cluster-network. It handles searching for clients, exchanging information and informing other objects about events.
    - Cluster-mutex: This is an object which can be used to synchronize access to a schared resource.
    - Cluster-container: This object allows to store objects in a container which is shared across the cluster network.
    - Cluster-database: Currently, this is the main object of the cluster network. Theis database can work with SQL statements and store/retrieve data across multiple nodes.

# Cluster-database
The cluster-database is described in more details because of the complexity. The cluster database has the following advantages:
 - Needs no configuration: The cluster-database needs not configuration because the clients use th p2p-network to search for other clients and ask for the current status.
 - Understands SQL: The SQL support is not yet fully implemented but some main features are already working.
 - Distributed data across nodes: The data of the database are stored in a distributed manner across all the nodes.
 - Redundancy: The cluster-database can be constructed by using a redundancy value. This means that there is no data loss as long as at least one client is online.
 - Hardware and operating system independent: The cluster-database currently runs on ARMv6, ARMv7, x86 and x86_64 devices. The database was successfully tested on Linux and Windows (At least those are the ones I tested...)

The following graphics illustrates the structure of the clustered database:
![cluster-database structure](images/structure.png?raw=true "Structure")
As already described above, the network layer is used for the communication between the nodes.
The other components are part of the network and have their own functionalities.
As shown in the picture, the database is built using two sub-components.
The serialization-component takes care of the correct order of packages and the distribution-component is resposible for storing the data across the nodes.

This makes it possible ot even use a different storage engine for the database.

## Scenario
The screenshot below shows the members of the cluster-network. The nodes in the upper left and upper right corners are Cubietrucks (ARMv7), the lower left corner is an IBM server (x86) and the node in the lower right corner is a raspberry PI (ARMv6).
At this point of the time the p2p network is already established which means that each node knows about the presence of others.

The screenshot also shows the structure of the cluster network.
The base element is the p2p-network, next is the database and the last element is a cluster-mutex which is used to synchronize some operations of the database.
