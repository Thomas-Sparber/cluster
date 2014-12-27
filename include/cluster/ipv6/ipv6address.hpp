#ifndef IPV6ADDRESS_HPP
#define IPV6ADDRESS_HPP

#include <cluster/prototypes/address.hpp>

namespace cluster
{

class IPv6Address : public Address
{

public:
	IPv6Address(const unsigned char a[16]);
	IPv6Address(const std::string &address);
	virtual Address* clone() const;
	virtual void increase();
	bool isLoopback() const;

	static void decode(const std::string &address, unsigned char a[16]);
	static std::string encode(const unsigned char a[16]);

protected:
	unsigned char a[16];


}; // end class IPv6Address

} //end namespace cluster

#endif
