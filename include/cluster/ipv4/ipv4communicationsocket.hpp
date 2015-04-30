/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef IPV4COMMUNICATIONSOCKET_HPP
#define IPV4COMMUNICATIONSOCKET_HPP

#include <cluster/prototypes/communicationsocket.hpp>

#ifndef __linux__
#include <winsock2.h>
#endif //__linux__

namespace cluster
{

class IPv4Address;

/**
  * This class is used to communicate with
  * another IPv4 client
 **/
class IPv4CommunicationSocket : public CommunicationSocket
{

public:
	/**
	  * Creates a communication socket to the
	  * client with the given Address. This constructor
	  * can be called when the connection was already
	  * established before.
	 **/
#ifdef __linux__
	IPv4CommunicationSocket(const IPv4Address &ipAddress, uint16_t port, int fd_client, unsigned int timeout);
#else
	IPv4CommunicationSocket(const IPv4Address &ipAddress, uint16_t port, SOCKET fd_client, unsigned int timeout);
#endif

	/**
	  * Creates a communication socket to the
	  * client with the given Address
	 **/
	IPv4CommunicationSocket(const IPv4Address &ipAddress, uint16_t port, unsigned int timeout);

	/**
	  * Copy constructor
	 **/
	IPv4CommunicationSocket(const IPv4CommunicationSocket &c);

	/**
	  * Assignment operator
	 **/
	IPv4CommunicationSocket& operator=(const IPv4CommunicationSocket &c);

	/**
	  * Default destructor
	 **/
	virtual ~IPv4CommunicationSocket();

	/**
	  * Sends the given Package to the target Address.
	 **/
	virtual bool send(const Package &data) override;

	/**
	  * Receives a Package from the target Address
	  * and stores is in out.
	 **/
	virtual bool receive(Package *out) override;

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
#ifdef __linux__
	int fd_client;
#else
	SOCKET fd_client;
#endif //__linux__

	/**
	  * This variable is used for the copy constructor
	  * and assignment operator to keep the socket open
	  * until all communication sockets are closed
	 **/
	int *counter;

}; // end class IPv4CommunicationSocket

} // end namespace cluster

#endif //IPV6COMMUNICATIONSOCKET_HPP
