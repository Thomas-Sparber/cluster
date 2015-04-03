/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include <cluster/prototypes/address.hpp>
#include <cluster/prototypes/communicationsocket.hpp>
#include <cluster/prototypes/listenersocket.hpp>
#include <string>
#include <list>

namespace cluster
{

class Package;

/**
  * This is the interface to a communication
  * protocol. Examples for this are IPv4 or IPv6.
  * This class provides all functions for creating
  * protocol specific objets such as Address,
  * CommunicationSocket or ListenerSocket
 **/
class Protocol
{

public:
	/**
	  * Default destructor
	 **/
	virtual ~Protocol() {}

	/**
	  * This function creates a ListenerSocket
	  * for the current Protocol
	 **/
	virtual ListenerSocket* createListenerSocket() const = 0;

	/**
	  * This function creates a ListenerSocket
	  * to the given Address for the current Protocol
	 **/
	virtual CommunicationSocket* createCommunicationSocket(const Address &address) const = 0;

	/**
	  * This function returns all addresses the computer
	  * has for the current Protocol
	 **/
	virtual void getAddresses(std::list<Address*> &out) const = 0;

	/**
	  * Decodes the given Address in string representation
	 **/
	virtual Address* decodeAddress(const std::string &address) const = 0;

}; // end class Protocol

} //end namespace cluster

#endif
