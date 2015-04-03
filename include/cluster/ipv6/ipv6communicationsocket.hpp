/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef IPV6COMMUNICATIONSOCKET_HPP
#define IPV6COMMUNICATIONSOCKET_HPP

#include <cluster/prototypes/communicationsocket.hpp>

namespace cluster
{

class IPv6Address;

/**
  * This class is used to communicate with
  * another IPv6 client
 **/
class IPv6CommunicationSocket : public CommunicationSocket
{

public:
	/**
	  * Creates a communication socket to the
	  * client with the given Address. This constructor
	  * can be called when the connection was already
	  * established before.
	 **/
	IPv6CommunicationSocket(const IPv6Address &ipAddress, uint16_t port, int fd_client, unsigned int timeout);

	/**
	  * Creates a communication socket to the
	  * client with the given Address
	 **/
	IPv6CommunicationSocket(const IPv6Address &ipAddress, uint16_t port, unsigned int timeout);

	/**
	  * Copy constructor
	 **/
	IPv6CommunicationSocket(const IPv6CommunicationSocket &c);

	/**
	  * Assignment operator
	 **/
	IPv6CommunicationSocket& operator=(const IPv6CommunicationSocket &c);

	/**
	  * Default destructor
	 **/
	virtual ~IPv6CommunicationSocket();

	/**
	  * Sends the given Package to the target Address.
	 **/
	virtual bool send(const Package &data);

	/**
	  * Receives a Package from the target Address
	  * and stores is in out.
	 **/
	virtual bool receive(Package *out);

	/**
	  * Returns the target port
	 **/
	uint16_t getPort() const
	{
		return port;
	}

private:
	/**
	  * The target port
	 **/
	uint16_t port;

	/**
	  * The file descriptor which is used by the socket
	 **/
	int fd_client;

	/**
	  * This variable is used for the copy constructor
	  * and assignment operator to keep the socket open
	  * until all communication sockets are closed
	 **/
	int *counter;

}; // end class IPv6CommunicationSocket

} //end namespace cluster

#endif //IPV6COMMUNCATIONSOCKET_HPP
