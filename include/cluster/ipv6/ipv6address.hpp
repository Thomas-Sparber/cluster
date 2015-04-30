/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef IPV6ADDRESS_HPP
#define IPV6ADDRESS_HPP

#include <cluster/prototypes/address.hpp>

namespace cluster
{

/**
  * This class represents an IPv6 address
 **/
class IPv6Address : public Address
{

public:
	/**
	  * Constructs an IPv4Address using a
	  * a char array
	 **/
	IPv6Address(const uint8_t a[16]);

	/**
	  * Constructs an address using its string
	  * representation
	 **/
	IPv6Address(const std::string &address);

	/**
	  * Creates a copy of the Address
	 **/
	virtual Address* clone() const override;

	/**
	  * Increases the Address by one interval
	  * this is used e.g. to scan address ranges
	 **/
	virtual void increase() override;

	/**
	  * Returns whether the current address
	  * is a loopback Address. This means that
	  * the Address is pointing to the current
	  * computer. e.g. 127.0.0.1
	 **/
	virtual bool isLoopback() const override;

	/**
	  * Converts an Address in string representation
	  * into the char array representation
	 **/
	static void decode(const std::string &address, uint8_t a[16]);

	/**
	  * Converts an Address in char array representation
	  * into the string representation
	 **/
	static std::string encode(const uint8_t a[16]);

protected:
	/**
	  * The address in char array representation
	 **/
	uint8_t a[16];


}; // end class IPv6Address

} //end namespace cluster

#endif
