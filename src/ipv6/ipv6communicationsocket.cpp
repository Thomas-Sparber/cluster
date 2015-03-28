#include <cluster/ipv6/ipv6communicationsocket.hpp>
#include <cluster/ipv6/ipv6address.hpp>
#include <cluster/package.hpp>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <assert.h>

using namespace std;
using namespace cluster;

IPv6CommunicationSocket::IPv6CommunicationSocket(const IPv6Address &ipAddress, uint16_t ui_port, int client, unsigned int timeout) :
	CommunicationSocket(ipAddress),
	port(ui_port),
	fd_client(client),
	counter(new int(1))
{

	struct timeval tv;
	tv.tv_sec = timeout;
	tv.tv_usec = 0;
	setsockopt(fd_client, SOL_SOCKET, SO_SNDTIMEO, static_cast<void*>(&tv), sizeof(struct timeval));
	setsockopt(fd_client, SOL_SOCKET, SO_RCVTIMEO, static_cast<void*>(&tv), sizeof(struct timeval));
}

IPv6CommunicationSocket::IPv6CommunicationSocket(const IPv6Address &ipAddress, uint16_t ui_port, unsigned int timeout) :
	CommunicationSocket(ipAddress),
	port(ui_port),
	fd_client(),
	counter(nullptr)
{
	fd_client = socket(AF_INET6, SOCK_STREAM, 0);
	if(fd_client == -1)
		throw CommunicationException(string("Unable to create socket: ")+strerror(errno));
	struct sockaddr_in6 addr;
	addr.sin6_family = AF_INET6;
	addr.sin6_port = htons(port);
	if(inet_pton(AF_INET6, ipAddress.address.c_str(), &addr.sin6_addr) != 1)
		throw CommunicationException(string("Couldn't convert ip address: ")+strerror(errno));
	if(connect(fd_client, (struct sockaddr*)&addr, sizeof(addr)) == -1)
		throw CommunicationException(string("Unable to connect to client: ")+strerror(errno));

	struct timeval tv;
	tv.tv_sec = timeout;
	tv.tv_usec = 0;
	setsockopt(fd_client, SOL_SOCKET, SO_SNDTIMEO, static_cast<void*>(&tv), sizeof(struct timeval));
	setsockopt(fd_client, SOL_SOCKET, SO_RCVTIMEO, static_cast<void*>(&tv), sizeof(struct timeval));

	counter = new int(1);
}


IPv6CommunicationSocket::IPv6CommunicationSocket(const IPv6CommunicationSocket &c) :
	CommunicationSocket(c),
	port(c.port),
	fd_client(c.fd_client),
	counter(c.counter)
{
	assert(counter != nullptr);
	assert(*counter >= 0);
	assert(fd_client >= 0);

	(*counter)++;
}

IPv6CommunicationSocket& IPv6CommunicationSocket::operator=(const IPv6CommunicationSocket &c)
{
	assert(c.counter != nullptr);
	assert(*counter >= 0);
	assert(c.fd_client >= 0);

	port = c.port;
	fd_client = c.fd_client;
	counter = c.counter;
	(*counter)++;
	return (*this);
}

IPv6CommunicationSocket::~IPv6CommunicationSocket()
{
	assert(counter != nullptr);
	assert(*counter >= 0);
	assert(fd_client >= 0);

	(*counter)--;
	if((*counter) == 0)
	{
		int err = -1;
		socklen_t len = sizeof(err);
		getsockopt(fd_client, SOL_SOCKET, SO_ERROR, (char*)err, &len);
		if(shutdown(fd_client, SHUT_RDWR) < 0)	//Terminate the reliable deilivery
		{
			if(err != ENOTCONN && err != EINVAL) {}	//Error
		}
		close(fd_client);
		delete counter;
	}
}

bool IPv6CommunicationSocket::send(const Package &message)
{
	assert(counter != nullptr);
	assert(*counter >= 0);
	assert(fd_client >= 0);

	unsigned int messageSize = message.getLength();
	if(write(fd_client, &messageSize, sizeof(messageSize)) != sizeof(messageSize))return false;
	if(write(fd_client, message.getData(), messageSize) != int(messageSize))return false;
	return true;
}

bool IPv6CommunicationSocket::receive(Package *out)
{
	assert(counter != nullptr);
	assert(*counter >= 0);
	assert(fd_client >= 0);

	unsigned int messageSize = 0;
	if(read(fd_client, &messageSize, sizeof(messageSize)) < 0)return false;
	char *data = new char[messageSize];
	if(read(fd_client, data, messageSize) < 0)
	{
		delete [] data;
		return false;
	}
	if(out)out->append(data, messageSize);
	delete [] data;
	return true;
}
