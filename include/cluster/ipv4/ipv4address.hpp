#ifndef IPV4ADDRESS_HPP
#define IPV4ADDRESS_HPP

#include <cluster/prototypes/address.hpp>

namespace cluster
{

class IPv4Address : public Address
{

public:
	IPv4Address(const unsigned char a[4]);
	IPv4Address(const std::string &address);
	virtual Address* clone() const;
	virtual void increase();
	bool isLoopback() const;

	static void decode(const std::string &address, unsigned char a[4]);
	static std::string encode(const unsigned char a[4]);

protected:
	unsigned char a[4];

}; // end class IPv4Address

} // end namespace cluster

#endif
