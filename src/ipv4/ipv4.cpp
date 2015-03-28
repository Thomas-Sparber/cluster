#include <cluster/ipv4/ipv4.hpp>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <iostream>

using namespace std;
using namespace cluster;

IPv4::IPv4(uint16_t ui_port, unsigned int ui_timeout) :
	Protocol(),
	port(ui_port),
	timeout(ui_timeout)
{}

IPv4::~IPv4()
{}

ListenerSocket* IPv4::createListenerSocket() const
{
	try {
		return new IPv4ListenerSocket(port, timeout);
	}catch(const ListenerException &e) {}

	return nullptr;
}

CommunicationSocket* IPv4::createCommunicationSocket(const Address &address) const
{
	try {
		const IPv4Address *a = static_cast<IPv4Address*>(decodeAddress(address.address));
		if(!a)throw AddressException("Wrong IPv4 Address");

		CommunicationSocket *socket = new IPv4CommunicationSocket(*a, port, timeout);
		delete a;
		return socket;
	}catch(const CommunicationException &e) {}

	return nullptr;
}

void IPv4::getAddresses(std::list<Address*> &out) const
{
	struct ifaddrs *ifAddrStruct = nullptr;

	getifaddrs(&ifAddrStruct);

	for(struct ifaddrs *ifa = ifAddrStruct; ifa != nullptr; ifa = ifa->ifa_next)
	{
		if(ifa->ifa_addr->sa_family == AF_INET)
		{
			// is a valid IP4 Address
			void *temp = reinterpret_cast<void*>(ifa->ifa_addr);
			void *tmpAddrPtr = &reinterpret_cast<struct sockaddr_in*>(temp)->sin_addr.s_addr;
			char addressBuffer[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
			if(Address *a = decodeAddress(addressBuffer))
			{
				out.push_back(a);
			}
		}
        }
        if(ifAddrStruct)freeifaddrs(ifAddrStruct);
}

Address* IPv4::decodeAddress(const std::string &address) const
{
	try {
		return new IPv4Address(address);
	} catch(const AddressException &e) {}

	return nullptr;
}
