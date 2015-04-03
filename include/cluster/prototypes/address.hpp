/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef ADDRESS_HPP
#define ADDRESS_HPP

#include <string>

namespace cluster
{

/**
  * This exception is thrown whenever an invalid
  * address is tried to build
 **/
class AddressException
{

public:
	/**
	  * Constructs the exception using a descritpion text
	 **/
	AddressException(const std::string &str_text) : text(str_text) {}

	/**
	  * The description of th exception
	 **/
	std::string text;

}; //end class AddressException

/**
  * This class is used to uniquely identify
  * a client in the network. Every protocol
  * uses its own addresses
 **/
class Address
{

public:
	/**
	  * Constructs an address using its string
	  * representation
	 **/
	Address(const std::string &str_address) : address(str_address) {}

	/**
	  * Default virtual destructor
	 **/
	virtual ~Address() {}

	/**
	  * Creates a copy of the Address
	 **/
	virtual Address* clone() const = 0;

	/**
	  * Increases the Address by one interval
	  * this is used e.g. to scan address ranges
	 **/
	virtual void increase() = 0;

	/**
	  * Returns if the Address is equel to
	  * the given Address
	 **/
	virtual bool operator== (const Address &a) const
	{
		return address == a.address;
	}

	/**
	  * Returns if the Address is equel to
	  * the given Address
	 **/
	virtual bool operator!= (const Address &a) const
	{
		return !((*this) == a);
	}

	/**
	  * Returns whether the current address
	  * is a loopback Address. This means that
	  * the Address is pointing to the current
	  * computer. e.g. 127.0.0.1
	 **/
	virtual bool isLoopback() const = 0;

public:
	/**
	  * Holds the string representation of the address
	 **/
	std::string address;

}; // end class Address

} //end namespace cluster

#endif //ADDRESS_HPP
