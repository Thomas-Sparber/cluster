/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#include <cluster/ipv4/ipv4.hpp>

#ifdef __linux__
#include <ifaddrs.h>
#include <arpa/inet.h>
#else
#include <winsock2.h>
#endif //__linux__

using namespace std;
using namespace cluster;

IPv4::IPv4(uint16_t ui_port, unsigned int ui_timeout, unsigned int ui_listenBacklog) :
	Protocol(),
	port(ui_port),
	timeout(ui_timeout),
	listenBacklog(ui_listenBacklog)
{
#ifndef __linux__
	WSAData data;
	WSAStartup(2, &data);
#endif //__linux__
}

IPv4::~IPv4()
{
#ifndef __linux__
	WSACleanup();
#endif //__linux__
}

ListenerSocket* IPv4::createListenerSocket() const
{
	try {
		return new IPv4ListenerSocket(port, timeout, listenBacklog);
	}catch(const ListenerException &e) {}

	return nullptr;
}

CommunicationSocket* IPv4::createCommunicationSocket(const Address &address) const
{
	CommunicationSocket *socket = nullptr;

	//Decode Address
	const IPv4Address *a = static_cast<IPv4Address*>(decodeAddress(address.address));
	if(!a)throw AddressException("Wrong IPv4 Address");

	try {
		socket = new IPv4CommunicationSocket(*a, port, timeout);
	}catch(const CommunicationException &e) {}

	delete a;
	return socket;
}

void IPv4::getAddresses(std::list<Address*> &out) const
{
	//Read addresses for current computer
#ifdef __linux__
	struct ifaddrs *ifAddrStruct = nullptr;
	getifaddrs(&ifAddrStruct);

	for(struct ifaddrs *ifa = ifAddrStruct; ifa != nullptr; ifa = ifa->ifa_next)
	{
		if(ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET)
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
#else
	char ac[256];
	if(gethostname(ac, sizeof(ac)) != SOCKET_ERROR)
	{
		if(struct hostent *phe = gethostbyname(ac))
		{
			for(unsigned int i = 0; phe->h_addr_list[i] != nullptr; ++i)
			{
				if(Address *a = decodeAddress(inet_ntoa(*reinterpret_cast<struct in_addr*>(phe->h_addr_list[i]))))
				{
					out.push_back(a);
				}
			}
		}
	}
#endif //__linux__
}

Address* IPv4::decodeAddress(const std::string &address) const
{
	try {
		return new IPv4Address(address);
	} catch(const AddressException &e) {}

	return nullptr;
}
