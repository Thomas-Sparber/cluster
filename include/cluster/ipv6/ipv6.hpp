#ifndef IPV6_HPP
#define IPV6_HPP

#include <cluster/prototypes/protocol.hpp>
#include <cluster/ipv6/ipv6address.hpp>
#include <cluster/ipv6/ipv6communicationsocket.hpp>
#include <cluster/ipv6/ipv6listenersocket.hpp>
#include <list>

namespace cluster
{

class IPv6 : public Protocol
{

public:
	IPv6(uint16_t port, unsigned int timeout=10);
	virtual ~IPv6();
	virtual ListenerSocket* createListenerSocket() const;
	virtual CommunicationSocket* createCommunicationSocket(const Address &address) const;
	virtual void getAddresses(std::list<Address*> &out) const;
	virtual Address* decodeAddress(const std::string &address) const;

private:
	uint16_t port;
	unsigned int timeout;

}; // end class IPv6

} //end namespace cluster

#endif
