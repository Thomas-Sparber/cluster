/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef IPV4ADDRESS_HPP
#define IPV4ADDRESS_HPP

#include <cluster/prototypes/address.hpp>

namespace cluster
{

/**
  * This class represents an IPv4 address
 **/
class IPv4Address : public Address
{

public:
	/**
	  * Constructs an IPv4Address using a
	  * a char array
	 **/
	IPv4Address(const unsigned char a[4]);

	/**
	  * Constructs an address using its string
	  * representation
	 **/
	IPv4Address(const std::string &address);

	/**
	  * Creates a copy of the Address
	 **/
	virtual Address* clone() const;

	/**
	  * Increases the Address by one interval
	  * this is used e.g. to scan address ranges
	 **/
	virtual void increase();

	/**
	  * Returns whether the current address
	  * is a loopback Address. This means that
	  * the Address is pointing to the current
	  * computer. e.g. 127.0.0.1
	 **/
	virtual bool isLoopback() const;

	/**
	  * Converts an Address in string representation
	  * into the char array representation
	 **/
	static void decode(const std::string &address, unsigned char a[4]);

	/**
	  * Converts an Address in char array representation
	  * into the string representation
	 **/
	static std::string encode(const unsigned char a[4]);

protected:
	/**
	  * The address in char array representation
	 **/
	unsigned char a[4];

}; // end class IPv4Address

} // end namespace cluster

#endif
