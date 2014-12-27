#ifndef IPV4_HPP
#define IPV4_HPP

#include <cluster/prototypes/protocol.hpp>
#include <cluster/ipv4/ipv4address.hpp>
#include <cluster/ipv4/ipv4communicationsocket.hpp>
#include <cluster/ipv4/ipv4listenersocket.hpp>
#include <list>

namespace cluster
{

class IPv4 : public Protocol
{

public:
	IPv4(uint16_t port, unsigned int timeout=10);
	virtual ~IPv4();
	virtual ListenerSocket* createListenerSocket() const;
	virtual CommunicationSocket* createCommunicationSocket(const Address &address) const;
	virtual void getAddresses(std::list<Address*> &out) const;
	virtual Address* decodeAddress(const std::string &address) const;

private:
	uint16_t port;
	unsigned int timeout;

	static const unsigned int LISTEN_BACKLOG = 50;

}; // end class IPv4

} //end namespace cluster

#endif
