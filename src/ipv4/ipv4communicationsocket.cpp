/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

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

IPv4CommunicationSocket::IPv4CommunicationSocket(const IPv4Address &ipAddress, uint16_t ui_port, int client, unsigned int timeout) :
	CommunicationSocket(ipAddress),
	port(ui_port),
	fd_client(client),
	counter(new int(1))
{
	//Set timeout
	struct timeval tv;
	tv.tv_sec = timeout;
	tv.tv_usec = 0;
	setsockopt(fd_client, SOL_SOCKET, SO_SNDTIMEO, static_cast<void*>(&tv), sizeof(struct timeval));
	setsockopt(fd_client, SOL_SOCKET, SO_RCVTIMEO, static_cast<void*>(&tv), sizeof(struct timeval));
}

IPv4CommunicationSocket::IPv4CommunicationSocket(const IPv4Address &ipAddress, uint16_t ui_port, unsigned int timeout) :
	CommunicationSocket(ipAddress),
	port(ui_port),
	fd_client(),
	counter(nullptr)
{
	//Open socket
	fd_client = socket(AF_INET, SOCK_STREAM, 0);
	if(fd_client == -1)
		throw CommunicationException(string("Unable to create socket: ")+strerror(errno));

	//Open port
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	if(inet_pton(AF_INET, ipAddress.address.c_str(), &addr.sin_addr.s_addr) != 1)
	{
		close(fd_client);
		throw CommunicationException(string("Couldn't convert ip address: ")+strerror(errno));
	}

	//Connect socket
	if(connect(fd_client, (struct sockaddr*)&addr, sizeof(addr)) == -1)
	{
		close(fd_client);
		throw CommunicationException(string("Unable to connect to client: ")+strerror(errno));
	}

	//Set timeout
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

	//Increase reference counter
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

	//Increase reference counter
	(*counter)++;

	return (*this);
}

IPv4CommunicationSocket::~IPv4CommunicationSocket()
{
	assert(counter != nullptr);
	assert(*counter >= 0);
	assert(fd_client >= 0);

	//Decrease reference counter
	(*counter)--;

	//If this class is the last one that connects
	//to the socket, close ist
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

bool IPv4CommunicationSocket::send(const Package &message)
{
	assert(counter != nullptr);
	assert(*counter >= 0);
	assert(fd_client >= 0);

	unsigned int messageSize = message.getLength();

	//First sending package size then package content

	//Sending 0 byte packages is illegal
	if(messageSize <= 0)
	{
		messageSize = 1;
		char data = '\0';
		if(::send(fd_client, &messageSize, sizeof(messageSize), MSG_NOSIGNAL) != sizeof(messageSize))return false;
		if(::send(fd_client, &data, messageSize, MSG_NOSIGNAL) != int(messageSize))return false;
	}
	else
	{
		if(::send(fd_client, &messageSize, sizeof(messageSize), MSG_NOSIGNAL) != sizeof(messageSize))return false;
		if(::send(fd_client, message.getData(), messageSize, MSG_NOSIGNAL) != int(messageSize))return false;
	}
	return true;
}

bool IPv4CommunicationSocket::receive(Package *out)
{
	assert(counter != nullptr);
	assert(*counter >= 0);
	assert(fd_client >= 0);

	//First reading package size then package content

	unsigned int messageSize = 0;
	if(read(fd_client, &messageSize, sizeof(messageSize)) < 0)return false;
	vector<unsigned char> data(messageSize);
	if(read(fd_client, &data[0], messageSize) < 0)return false;
	if(out)out->append(&data[0], messageSize);
	return true;
}
