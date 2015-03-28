#ifndef ADDRESS_HPP
#define ADDRESS_HPP

#include <string>

namespace cluster
{

class AddressException
{

public:
	AddressException(const std::string &str_text) : text(str_text) {}
	std::string text;

}; //end class AddressException

class Address
{

public:
	Address(const std::string &str_address) : address(str_address) {}
	virtual ~Address() {}

	virtual Address* clone() const = 0;
	virtual void increase() = 0;
	virtual bool operator==(const Address &a) const { return address == a.address; }
	virtual bool operator!=(const Address &a) const { return !((*this) == a); }
	virtual bool isLoopback() const = 0;

public:
	std::string address;

}; // end class Address

} //end namespace cluster

#endif //ADDRESS_HPP
