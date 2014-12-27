#include <cluster/ipv4/ipv4communicationsocket.hpp>
#include <cluster/ipv4/ipv4address.hpp>
#include <cluster/package.hpp>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <assert.h>

using namespace std;
using namespace cluster;

IPv4CommunicationSocket::IPv4CommunicationSocket(const IPv4Address &ipAddress, uint16_t port, int fd_client, unsigned int timeout) :
	CommunicationSocket(ipAddress),
	port(port),
	fd_client(fd_client),
	counter(new int(1))
{
	struct timeval tv;
	tv.tv_sec = timeout;
	tv.tv_usec = 0;
	setsockopt(fd_client, SOL_SOCKET, SO_SNDTIMEO, static_cast<void*>(&tv), sizeof(struct timeval));
	setsockopt(fd_client, SOL_SOCKET, SO_RCVTIMEO, static_cast<void*>(&tv), sizeof(struct timeval));
}

IPv4CommunicationSocket::IPv4CommunicationSocket(const IPv4Address &ipAddress, uint16_t port, unsigned int timeout) :
	CommunicationSocket(ipAddress),
	port(port),
	fd_client(),
	counter(nullptr)
{
	fd_client = socket(AF_INET, SOCK_STREAM, 0);
	if(fd_client == -1)
		throw CommunicationException(string("Unable to create socket: ")+strerror(errno));
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	if(inet_pton(AF_INET, ipAddress.address.c_str(), &addr.sin_addr.s_addr) != 1)
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


IPv4CommunicationSocket::IPv4CommunicationSocket(const IPv4CommunicationSocket &c) :
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

IPv4CommunicationSocket& IPv4CommunicationSocket::operator=(const IPv4CommunicationSocket &c)
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

IPv4CommunicationSocket::~IPv4CommunicationSocket()
{
	assert(counter != nullptr);
	assert(*counter >= 0);
	assert(fd_client >= 0);

	(*counter)--;
	if((*counter) == 0)
	{
		close(fd_client);
		delete counter;
	}
}
//#include <iostream>
bool IPv4CommunicationSocket::send(const Package &message)
{
	//cout<<"Sending "<<message.toString()<<endl;
	assert(counter != nullptr);
	assert(*counter >= 0);
	assert(fd_client >= 0);

	unsigned int messageSize = message.getLength();
	if(write(fd_client, &messageSize, sizeof(messageSize)) == -1)return false;
	if(write(fd_client, message.getData(), messageSize) == -1)return false;
	return true;
}

bool IPv4CommunicationSocket::receive(Package *out)
{
	assert(counter != nullptr);
	assert(*counter >= 0);
	assert(fd_client >= 0);

	unsigned int messageSize = 0;
	if(read(fd_client, &messageSize, sizeof(messageSize)) == -1)return false;
	char *data = new char[messageSize];
	if(read(fd_client, data, messageSize) == -1)
	{
		delete [] data;
		return false;
	}
	if(out){
		out->append(*data, messageSize);
		//cout<<"Received "<<out->toString()<<endl;
	}
	delete [] data;
	return true;
}
