/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef IPV4LISTENERSOCKET_HPP
#define IPV4LISTENERSOCKET_HPP

#include <cluster/prototypes/listenersocket.hpp>

#ifndef __linux__
#include <winsock2.h>
#endif //__linux__

namespace cluster
{

class CommunicationSocket;

/**
  * This class can be used to listen for
  * client connections.
 **/
class IPv4ListenerSocket : public ListenerSocket
{

public:
	/**
	  * Creates a listener socket which listens on
	  * the given port. The listenBacklog defines how
	  * many IP packages should be stored before they
	  * are processed. Timeout is used when a client
	  * connection is established
	 **/
	IPv4ListenerSocket(uint16_t port, unsigned int timeout, unsigned int listenBacklog);

	/**
	  * Default destructor
	 **/
	virtual ~IPv4ListenerSocket();

	/**
	  * Cheks for the given timesout if a
	  * client is waiting for a connection.
	 **/
	virtual bool poll(unsigned int time) override;

	/**
	  * Accepts the client connection. Doesn't
	  * return until the connection is established
	 **/
	virtual CommunicationSocket* listen() override;

	/**
	  * Returns the port on which to listen
	 **/
	uint16_t getPort() const
	{
		return port;
	}

	/**
	  * Returns the timeout which is used when
	  * establishing client connections
	 **/
	unsigned int getTimeout() const
	{
		return timeout;
	}

private:
	/**
	  * The port on which to listen
	 **/
	uint16_t port;

	/**
	  * The server socket
	 **/
#ifdef __linux__
	int fd_socket;
#else
	SOCKET fd_socket;
#endif //__linux__

	/**
	  * The timeout which is used when establishing
	  * client connections
	 **/
	unsigned int timeout;

}; // end class IPv4ListenerSocket

} // end namespace cluster

#endif //IPV4LISTENERSOCKET_HPP
