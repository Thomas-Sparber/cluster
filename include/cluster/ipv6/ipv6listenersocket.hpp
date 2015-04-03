/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef IPV6LISTENERSOCKET_HPP
#define IPV6LISTENERSOCKET_HPP

#include <cluster/prototypes/listenersocket.hpp>

namespace cluster
{

class CommunicationSocket;

/**
  * This class can be used to listen for
  * client connections.
 **/
class IPv6ListenerSocket : public ListenerSocket
{

public:
	/**
	  * Creates a listener socket which listens on
	  * the given port. The listenBacklog defines how
	  * many IP packages should be stored before they
	  * are processed. Timeout is used when a client
	  * connection is established
	 **/
	IPv6ListenerSocket(uint16_t port, unsigned int timeout, unsigned int listenBacklog);

	/**
	  * Default destructor
	 **/
	virtual ~IPv6ListenerSocket();

	/**
	  * Cheks for the given timesout if a
	  * client is waiting for a connection.
	 **/
	virtual bool poll(unsigned int time);

	/**
	  * Accepts the client connection. Doesn't
	  * return until the connection is established
	 **/
	virtual CommunicationSocket* listen();

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
	int fd_socket;

	/**
	  * The timeout which is used when establishing
	  * client connections
	 **/
	unsigned int timeout;

}; // end class IPv6ListenerSocket

} //end namespace cluster

#endif //IPV6LISTENERSOCKET_HPP
