#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include <cluster/prototypes/address.hpp>
#include <cluster/prototypes/communicationsocket.hpp>
#include <cluster/prototypes/listenersocket.hpp>
#include <string>
#include <list>

namespace cluster
{

class Package;

class Protocol
{

public:
	Protocol() {}
	virtual ~Protocol() {}
	virtual ListenerSocket* createListenerSocket() const = 0;
	virtual CommunicationSocket* createCommunicationSocket(const Address &address) const = 0;
	virtual void getAddresses(std::list<Address*> &out) const = 0;
	virtual Address* decodeAddress(const std::string &address) const = 0;

}; // end class Protocol

} //end namespace cluster

#endif
