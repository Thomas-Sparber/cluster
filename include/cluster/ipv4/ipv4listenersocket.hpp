#ifndef IPV4LISTENERSOCKET_HPP
#define IPV4LISTENERSOCKET_HPP

#include <cluster/prototypes/listenersocket.hpp>

namespace cluster
{

class CommunicationSocket;

class IPv4ListenerSocket : public ListenerSocket
{

public:
	IPv4ListenerSocket(uint16_t port, unsigned int timeout);
	virtual ~IPv4ListenerSocket();
	virtual bool poll(unsigned int time);
	virtual CommunicationSocket* listen();

private:
	uint16_t port;
	int fd_socket;
	unsigned int timeout;

}; // end class IPv4ListenerSocket

} // end namespace cluster

#endif //IPV4LISTENERSOCKET_HPP
