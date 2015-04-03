/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef COMMUNICATIONSOCKET_HPP
#define COMMUNICATIONSOCKET_HPP

#include "address.hpp"

namespace cluster
{

class Package;

/**
  * This exception is thrown whenever a connection
  * can not be established
 **/
class CommunicationException
{

public:
	/**
	  * Constructs the eeception with the given
	  * description text
	 **/
	CommunicationException(const std::string &str_text) : text(str_text) {}

	/**
	  * The description of the exception
	 **/
	std::string text;

}; //end class CommunicationException

/**
  * This class is used to communicate with
  * another client
 **/
class CommunicationSocket
{

public:
	/**
	  * Creates a communication socket to the
	  * client with the given Address
	 **/
	CommunicationSocket(const Address &a) :
		address(a.clone())
	{}

	/**
	  * Copy constructor
	 **/
	CommunicationSocket(const CommunicationSocket &c) :
		address(c.address->clone())
	{}

	/**
	  * Assignment operator
	 **/
	CommunicationSocket& operator=(const CommunicationSocket &c)
	{
		delete this->address;
		this->address = c.address->clone();
		return (*this);
	}

	/**
	  * Default destructor
	 **/
	virtual ~CommunicationSocket()
	{
		delete address;
	}

	/**
	  * Returns address to communicate with
	 **/
	const Address& getAddress() const
	{
		return *address;
	}

	/**
	  * Sends the given Package to the target Address.
	 **/
	virtual bool send(const Package &data) = 0;

	/**
	  * Receives a Package from the target Address
	  * and stores is in out.
	 **/
	virtual bool receive(Package *out) = 0;

protected:
	/**
	  * The target address to communicate with
	 **/
	const Address *address;

}; // end class ListenerSocket

} // end namespace cluster

#endif //COMMUNICATIONSOCKET_HPP
