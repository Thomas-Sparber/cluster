#ifndef IPV6COMMUNICATIONSOCKET_HPP
#define IPV6COMMUNICATIONSOCKET_HPP

#include <cluster/prototypes/communicationsocket.hpp>

namespace cluster
{

class IPv6Address;

class IPv6CommunicationSocket : public CommunicationSocket
{

public:
	IPv6CommunicationSocket(const IPv6Address &ipAddress, uint16_t port, int fd_client, unsigned int timeout);
	IPv6CommunicationSocket(const IPv6Address &ipAddress, uint16_t port, unsigned int timeout);
	IPv6CommunicationSocket(const IPv6CommunicationSocket &c);
	IPv6CommunicationSocket& operator=(const IPv6CommunicationSocket &c);
	virtual ~IPv6CommunicationSocket();
	virtual bool send(const Package &data);
	virtual bool receive(Package *out);

private:
	uint16_t port;
	int fd_client;
	int *counter;

}; // end class IPv6CommunicationSocket

} //end namespace cluster

#endif //IPV6COMMUNCATIONSOCKET_HPP
