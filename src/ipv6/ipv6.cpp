/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#include <cluster/ipv6/ipv6.hpp>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>

using namespace std;
using namespace cluster;

IPv6::IPv6(uint16_t ui_port, unsigned int ui_timeout, unsigned int ui_listenBacklog) :
	Protocol(),
	port(ui_port),
	timeout(ui_timeout),
	listenBacklog(ui_listenBacklog)
{}

IPv6::~IPv6()
{}

ListenerSocket* IPv6::createListenerSocket() const
{
	try {
		return new IPv6ListenerSocket(port, timeout, listenBacklog);
	}catch(const ListenerException &e) {}

	return nullptr;
}

CommunicationSocket* IPv6::createCommunicationSocket(const Address &address) const
{
	try {
		//Decode Address
		const IPv6Address *a = static_cast<IPv6Address*>(decodeAddress(address.address));
		if(!a)throw AddressException("Wrong IPv6 Address");

		CommunicationSocket *socket = new IPv6CommunicationSocket(*a, port, timeout);
		delete a;
		return socket;
	}catch(const CommunicationException &e) {}

	return nullptr;
}

void IPv6::getAddresses(std::list<Address*> &out) const
{
	//Read addresses for current computer
	struct ifaddrs *ifAddrStruct = nullptr;
	getifaddrs(&ifAddrStruct);

	for(struct ifaddrs *ifa = ifAddrStruct; ifa != nullptr; ifa = ifa->ifa_next)
	{
		if(ifa->ifa_addr->sa_family == AF_INET6)
		{
			// is a valid IP6 Address
			void *temp = reinterpret_cast<void*>(ifa->ifa_addr);
			void *tmpAddrPtr = &reinterpret_cast<struct sockaddr_in6*>(temp)->sin6_addr;
			char addressBuffer[INET6_ADDRSTRLEN];
			inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
			if(Address *a = decodeAddress(addressBuffer))
			{
				out.push_back(a);
			}
		}
        }
        if(ifAddrStruct)freeifaddrs(ifAddrStruct);
}

Address* IPv6::decodeAddress(const std::string &address) const
{
	try {
		return new IPv6Address(address);
	} catch(const AddressException &e) {
		try {
			return new IPv6Address(string("::") + address);
		} catch(const AddressException &ex) {}
	}

	return nullptr;
}
