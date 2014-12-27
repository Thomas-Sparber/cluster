#ifndef IPV6LISTENERSOCKET_HPP
#define IPV6LISTENERSOCKET_HPP

#include <cluster/prototypes/listenersocket.hpp>

namespace cluster
{

class CommunicationSocket;

class IPv6ListenerSocket : public ListenerSocket
{

public:
	IPv6ListenerSocket(uint16_t port, unsigned int timeout);
	virtual ~IPv6ListenerSocket();
	virtual bool poll(unsigned int time);
	virtual CommunicationSocket* listen();

private:
	uint16_t port;
	int fd_socket;
	unsigned int timeout;

}; // end class IPv6ListenerSocket

} //end namespace cluster

#endif //IPV6LISTENERSOCKET_HPP
