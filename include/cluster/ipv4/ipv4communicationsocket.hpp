#ifndef IPV4COMMUNICATIONSOCKET_HPP
#define IPV4COMMUNICATIONSOCKET_HPP

#include <cluster/prototypes/communicationsocket.hpp>

namespace cluster
{

class IPv4Address;

class IPv4CommunicationSocket : public CommunicationSocket
{

public:
	IPv4CommunicationSocket(const IPv4Address &ipAddress, uint16_t port, int fd_client, unsigned int timeout);
	IPv4CommunicationSocket(const IPv4Address &ipAddress, uint16_t port, unsigned int timeout);
	IPv4CommunicationSocket(const IPv4CommunicationSocket &c);
	IPv4CommunicationSocket& operator=(const IPv4CommunicationSocket &c);
	virtual ~IPv4CommunicationSocket();
	virtual bool send(const Package &data);
	virtual bool receive(Package *out);

private:
	uint16_t port;
	int fd_client;
	int *counter;

}; // end class IPv4CommunicationSocket

} // end namespace cluster

#endif //IPV6COMMUNICATIONSOCKET_HPP
