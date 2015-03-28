#include <cluster/ipv6/ipv6address.hpp>
#include <ctype.h>
#include <arpa/inet.h>

using namespace std;
using namespace cluster;

IPv6Address::IPv6Address(const unsigned char uc_a[16]) :
	Address(IPv6Address::encode(uc_a)),
	a()
{
	for(unsigned int i = 0; i < 16; i++)
	{
		this->a[i] = uc_a[i];
	}
}

IPv6Address::IPv6Address(const string &str_address) :
	Address(str_address),
	a()
{
	decode(this->address, a);
}

void IPv6Address::decode(const std::string &address, unsigned char a[16])
{
	struct in6_addr addr;
	if(inet_pton(AF_INET6, address.c_str(), &addr) != 1)
	{
		throw AddressException("IPv6 address not valid");
	}
	for(unsigned int i = 0; i < 16; i++)
	{
		a[i] = addr.s6_addr[i];
	}
}

std::string IPv6Address::encode(const unsigned char a[16])
{
	struct in6_addr addr;
	for(unsigned int i = 0; i < 16; i++)
	{
		addr.s6_addr[i] = a[i];
	}
	char addressBuffer[INET6_ADDRSTRLEN];
	inet_ntop(AF_INET6, &addr, addressBuffer, INET6_ADDRSTRLEN);
	return addressBuffer;
}

Address* IPv6Address::clone() const
{
	return new IPv6Address(*this);
}

void IPv6Address::increase()
{
	bool success = false;
	for(int counter = 15; counter >= 0; counter--)
	{
		if(counter == 15)
		{
			if(a[counter] < 254)
			{
				a[counter]++;
				success = true;
				break;
			}
			else a[counter] = 1;
		}
		else
		{
			if(a[counter] < 255)
			{
				a[counter]++;
				success = true;
				break;
			}
			else a[counter] = 0;
		}
	}

	if(!success)
	{
		for(unsigned int i = 0; i < 15; i++)
		{
			a[i] = 0;
		}
		a[15] = 1;
	}

	address = encode(a);
}

bool IPv6Address::isLoopback() const
{
	if(a[15] != 1)return false;
	for(unsigned int i = 0; i < 15; i++)
	{
		if(a[i] != 0)return false;
	}
	return true;
}
