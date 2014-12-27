#ifndef CLUSTEROBJECTSERIALIZED_HPP
#define CLUSTEROBJECTSERIALIZED_HPP

#include <cluster/clusterobject.hpp>
#include <cluster/prototypes/membercallback.hpp>
#include <list>

namespace cluster
{

class ClusterObjectSerialized : public ClusterObject, public MemberCallback
{

private:
	enum ClusterObjectSerializedMessageType : unsigned char
	{
		message_other = 'o',
		message_mine = 'm'
	};

public:
	ClusterObjectSerialized(ClusterObject *network, unsigned int maxPackagesToRemember=100) :
		ClusterObject(network),
		lastPackages(),
		maxPackagesToRemember(maxPackagesToRemember)
	{
		addMemberCallback(this);
	}

	~ClusterObjectSerialized()
	{
		removeMemberCallback(this);
	}

protected:
	virtual void memberOnline(const Address &ip, bool isMaster)
	{
		//Only read last actions if new member is master
		//This means that it contains more members of the P2P
		//network as we do.
		if(!isMaster)return;

		//Ask master for list of last actions
	}

	virtual void memberOffline(const Address __attribute__((__unused__)) &ip) {}

	virtual bool received(const Address __attribute__((__unused__)) &ip, const Package &message, Package __attribute__((__unused__)) &answer, Package  __attribute__((__unused__)) &send)
	{
		ClusterObjectSerializedMessageType type;
		message>>type;
		if(type == message_mine)
		{
			return true;
		}
		else return false;
	}

	virtual void sendPackage(const Package &a, Package *answer)
	{
		lastPackages.push_back(Package(a, true));
		while(lastPackages.size() > maxPackagesToRemember)lastPackages.pop_front();

		Package message;
		message<<message_other;
		message<<a;
		ClusterObject::ClusterObject_send(addCurrentSignature(message), answer);
	}

	virtual bool askPackage(const Address &ip, const Package &a, Package *answer)
	{
		Package message;
		message<<message_other;
		message<<a;
		return ClusterObject::ClusterObject_ask(ip, addCurrentSignature(message), answer);
	}

private:
	std::list<Package> lastPackages;
	unsigned int maxPackagesToRemember;

}; //end class ClusterObjectSerialized

} //end namespace cluster

#endif //CLUSTEROBJECTSERIALIZED_HPP
