/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef P2P_HPP
#define P2P_HPP

#include <cluster/prototypes/protocol.hpp>
#include <cluster/package.hpp>
#include <cluster/clusterobject.hpp>
#include <list>
#include <thread>
#include <mutex>

namespace cluster
{

class Address;
class Client;
class Server;
class MemberCallback;

/**
  * The p2p (Peer-to-Peer) network contains
  * all the logic to create a decentralised
  * network with members of the same Protocol.
 **/
class p2p : public ClusterObject
{

public:
	/**
	  * Constructs a Peer-to-Peer network using
	  * the given protocol
	 **/
	p2p(const Protocol &p);

	/**
	  * Default destructor which closes all connections
	 **/
	~p2p();

	/**
	  * Opens the server connection and starts the update thread
	 **/
	void open();

	/**
	  * Closes the server connection and the update thread
	 **/
	void close();

	/**
	  * Overrides the function from ClusterObject
	  * and adds the given MemberCallback to the list of
	  * member callbacks.
	 **/
	virtual void addMemberCallback(MemberCallback *memberCallback) override
	{
		this->memberCallbacks.push_back(memberCallback);
	}

	/**
	  * Overrides the function from ClusterObject
	  * and removes the given MemberCallback from the list of
	  * member callbacks.
	 **/
	virtual void removeMemberCallback(MemberCallback *memberCallback) override
        {
		this->memberCallbacks.remove(memberCallback);
        }

	/**
	  * Returns the type of ClusterObject
	 **/
	virtual std::string getType() const override
	{
		return "P2P";
	}

	/**
	  * Returns the current amount of memebers of the p2p
	  * network
	 **/
	unsigned int getMembersCount() const
	{
		return members.size();
	}

	/**
	  * Adds an address range for members to scan
	 **/
	void addAddressRange(const Address &start, const Address &end);

	/**
	  * Checks if the given address points to the
	  * current computer
	 **/
	bool isOwnAddress(const Address &a) const;

	/**
	  * Checks if the given client is a member of the
	  * p2p network
	 **/
	bool isMember(const Client &client);

	/**
	  * Checks if a memeber has the given ip address
	 **/
	bool isMember(const Address &address);

	/**
	  * Sets the number of reconnect retries when
	  * a member goes offline
	 **/
	void setReconnectRetries(unsigned int ui_reconnectRetries)
	{
		this->reconnectRetries = ui_reconnectRetries;
	}

	/**
	  * Gets the number of reconnect retries when
	  * a member goes offline
	 **/
	unsigned int getReconnectRetries() const
	{
		return this->reconnectRetries;
	}

	/**
	  * Sets whether to continue working when the p2p
	  * network has no members
	 **/
	void setContinueWithoutMembers(bool b_continueWithoutMembers)
	{
		this->continueWithoutMembers = b_continueWithoutMembers;
	}

	/**
	  * Gets whether to continue working when the p2p
	  * network has no members
	 **/
	bool getContinueWithoutMembers() const
	{
		return this->continueWithoutMembers;
	}

protected:
	/**
	  * This function is called for every Package.
	 **/
	virtual bool received(const Address &ip, const Package &message, Package &answer, Package &send) override;

private:
	/**
	  * Copying a P2P network is invalid!
	 **/
	p2p(const p2p &p);

	/**
	  * Copying a P2P network is invalid!
	 **/
	p2p& operator=(const p2p &p);

	/**
	  * Creates the listener server
	 **/
	void createServer();

	/**
	  * This function is called for every Package.
	  * This function gives the package to the parent
	  * class ClusterObject which distributes it to
	  * the corrent ClusterObject
	 **/
	void received_internal(const Address &ip, const Package &message, Package &answer);

	/**
	  * This function is called by the testAlive thread
	  * to search for new members
	 **/
        void connectToHosts();

	/**
	  * This function is called by the testAliveThread
	  * to test the connection to a specific address
	 **/
	bool testConnection(const Address &ip, unsigned int retry);

	/**
	  * This function is called internally whenever a member
	  * is online. This fucntion asks then for other peers
	  * and calls the member callbacks
	 **/
	void online(const Address &address, unsigned long long otherTime);

	/**
	  * This function is called internally whenever a member
	  * is online. This fucntion calls the member callbacks
	 **/
	void offline(const Address &address);

	/**
	  * Overrides the function from ClusterObject.
	  * This is the final function of the network
	  * when a member is asked. This function actually asks
	  * the member
	 **/
	virtual bool ClusterObject_ask(const Address &ip, const Package &message, Package *answer) override;

	/**
	  * Overrides the function from ClusterObject.
	  * This is the final function of the network
	  * when a package is snet. This function actually
	  *sends the package
	 **/
	virtual bool ClusterObject_send(const Package &message, AnswerPackage *answer) override;

private:
	/**
	  * Allows parallel access to the addressRanges list
	 **/
	std::mutex addressRangeMutex;

	/**
	  * Contains the address reanges which should be scanned.
	 **/
	std::list<std::pair<Address*,Address*> > addressRanges;

	/**
	  * The protocol which is used for communication
	 **/
	const Protocol &protocol;

	/**
	  * A flag that determines whether the p2p network
	  * should is connected and therefore should search
	  * for other peers.
	 **/
	bool isConnected;

	/**
	  * This list stores all the clients which were found
	  * but didn't respond their peers yet. This is very
	  * important to know to keep synchronized. The p2p
	  * network needs to send a message to more than the
	  * half of the clients. this can only be assured when
	  * the peers are synchronized
	 **/
	std::list<Client> otherPeersToCheck;

	/**
	  * This mutex allows parallel access to otherPeersToCheck
	 **/
	std::mutex otherPeersToCheckMutex;

	/**
	  * Contains the addresses of the current computer. This is
	  * used to avoid connections to the local host
	 **/
	std::list<Address*> addresses;

	/**
	  * The members of the p2p network
	 **/
	std::list<Client> members;

	/**
	  * The server which listens for connections
	 **/
	Server* server;

	/**
	  * The test which searches for other peers
	 **/
	std::thread *testAliveThread;

	/**
	  * The mutex wich allows parallel access to the member list 
	 **/
	std::mutex memberMutex;

	/**
	  * Contains all the memebr callbacks which are notified
	  * whenever a member goes online or offline
	 **/
	std::list<MemberCallback*> memberCallbacks;

	/**
	  * The start time of the p2p network which is used
	  * to determine the "master" host.
	 **/
	unsigned long long startTime;


	/**
	  * The number of reconnect retries when
	  * a member goes offline
	 **/
	unsigned int reconnectRetries;

	/**
	  * A flag whether to continue working when the p2p
	  * network has no members
	 **/
	bool continueWithoutMembers;

}; // end class p2p

} //end namespace cluster

#endif
