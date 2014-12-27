#ifndef P2P_HPP
#define P2P_HPP

#include <cluster/prototypes/protocol.hpp>
#include <cluster/package.hpp>
#include <cluster/clusterobject.hpp>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>

namespace cluster
{

class Address;
class Client;
class Server;
class MemberCallback;

class p2p : public ClusterObject
{

public:
	p2p(const Protocol &p);
	~p2p();

	virtual void addMemberCallback(MemberCallback *memberCallback)
	{
		this->memberCallbacks.push_back(memberCallback);
	}

	virtual void removeMemberCallback(MemberCallback *memberCallback)
        {
		this->memberCallbacks.remove(memberCallback);
        }

	virtual std::string getType() const
	{
		return "P2P";
	}

	unsigned int getMembersCount() const;

	void addAddressRange(const Address &start, const Address &end);

	bool isOwnAddress(const Address &a) const;

	bool isMember(const Client &client);

	bool isMember(const Address &address);

protected:
	virtual bool received(const Address &ip, const Package &message, Package &answer, Package &send);

private:
	//Copying a P2P network is invalid!
	p2p(const p2p &p);
	p2p& operator=(const p2p &p);

	void createServer();
	void received_internal(const Address &ip, const Package &message, Package &answer);
        void connectToHosts();
	bool testConnection(const Address &ip, unsigned int retry);
	void checkForNewMembers();
	void online(const Address &address, unsigned int memb);
	void offline(const Address &address);
	virtual bool ClusterObject_ask(const Address &ip, const Package &message, Package *answer);
	virtual void ClusterObject_send(const Package &message, Package *answer);

private:
	std::mutex addressRangeMutex;
	std::list<std::pair<Address*,Address*> > addressRanges;
	const Protocol &protocol;
	bool isConnected;
	std::list<Address*> addresses;
	std::list<Client> members;
	Server* server;
	std::thread *testAliveThread;
	std::mutex memberMutex;
	std::list<MemberCallback*> memberCallbacks;

}; // end class p2p

} //end namespace cluster

#endif
