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
#include <unistd.h>

#ifdef __linux__
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cluster/windowshelpers.hpp>
#define SHUT_RDWR SD_BOTH
#endif //__linux__

using namespace std;
using namespace cluster;

#ifdef __linux__
IPv4CommunicationSocket::IPv4CommunicationSocket(const IPv4Address &ipAddress, uint16_t ui_port, int client, unsigned int timeout) :
#else
IPv4CommunicationSocket::IPv4CommunicationSocket(const IPv4Address &ipAddress, uint16_t ui_port, SOCKET client, unsigned int timeout) :
#endif //__linux__
	CommunicationSocket(ipAddress),
	port(ui_port),
	fd_client(client),
	counter(new int(1))
{
	//Set timeout
	struct timeval tv;
	tv.tv_sec = timeout;
	tv.tv_usec = 0;
	setsockopt(fd_client, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<char*>(&tv), sizeof(struct timeval));
	setsockopt(fd_client, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char*>(&tv), sizeof(struct timeval));

//#ifndef __linux__
//	//Set flag not to send broken pipe message
//	int set = 1;
//	setsockopt(fd_client, SOL_SOCKET, SO_NOSIGPIPE, reinterpret_cast<char*>(&set), sizeof(set));
//#endif //__linux__
}

IPv4CommunicationSocket::IPv4CommunicationSocket(const IPv4Address &ipAddress, uint16_t ui_port, unsigned int timeout) :
	CommunicationSocket(ipAddress),
	port(ui_port),
	fd_client(),
	counter(nullptr)
{
	//Open socket
	fd_client = socket(AF_INET, SOCK_STREAM, 0);
#ifdef __linux__
	if(fd_client == -1)
		throw CommunicationException(string("Unable to create socket: ")+strerror(errno));
#else
	if(fd_client == INVALID_SOCKET)
		throw CommunicationException(string("Unable to create socket: ")+to_string(WSAGetLastError()));
#endif //__linux__

	//Open port
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = uint16_t(htons(port));
	if(inet_pton(AF_INET, ipAddress.address.c_str(), &addr.sin_addr.s_addr) != 1)
	{
#ifdef __linux__
		close(fd_client);
#else
		closesocket(fd_client);
#endif //__linux__
		throw CommunicationException(string("Couldn't convert ip address"));
	}

	//Connect socket
	if(connect(fd_client, (struct sockaddr*)&addr, sizeof(addr)) != 0)
	{
#ifdef __linux__
		close(fd_client);
		throw CommunicationException(string("Unable to connect to client: ")+strerror(errno));
#else
		closesocket(fd_client);
		throw CommunicationException(string("Unable to connect to client: ")+to_string(WSAGetLastError()));
#endif //__linux__
	}

	//Set timeout
	struct timeval tv;
	tv.tv_sec = timeout;
	tv.tv_usec = 0;
	setsockopt(fd_client, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<char*>(&tv), sizeof(struct timeval));
	setsockopt(fd_client, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<char*>(&tv), sizeof(struct timeval));

//#ifndef __linux__
//	//Set flag not to send broken pipe message
//	int set = 1;
//	setsockopt(fd_client, SOL_SOCKET, SO_NOSIGPIPE, reinterpret_cast<char*>(&set), sizeof(set));
//#endif //__linux__

	counter = new int(1);
}


IPv4CommunicationSocket::IPv4CommunicationSocket(const IPv4CommunicationSocket &c) :
	CommunicationSocket(c),
	port(c.port),
	fd_client(c.fd_client),
	counter(c.counter)
{
	//Increase reference counter
	(*counter)++;
}

IPv4CommunicationSocket& IPv4CommunicationSocket::operator=(const IPv4CommunicationSocket &c)
{
	port = c.port;
	fd_client = c.fd_client;
	counter = c.counter;

	//Increase reference counter
	(*counter)++;

	return (*this);
}

IPv4CommunicationSocket::~IPv4CommunicationSocket()
{
	//Decrease reference counter
	(*counter)--;

	//If this class is the last one that connects
	//to the socket, close ist
	if((*counter) == 0)
	{
		int err = -1;
#ifdef __linux__
		socklen_t len = sizeof(err);
#else
		int len = sizeof(err);
#endif //__linux__
		getsockopt(fd_client, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&err), &len);
		if(shutdown(fd_client, SHUT_RDWR) < 0)	//Terminate the reliable deilivery
		{
			if(err != ENOTCONN && err != EINVAL) {}	//Error
		}
#ifdef __linux__
		close(fd_client);
#else
		closesocket(fd_client);
#endif //__linux__
		delete counter;
	}
}

bool IPv4CommunicationSocket::send(const Package &message)
{
#ifndef __linux__
	//Fake flag.
	//This flag is not needed in windows because
	//SO_NOSIGPIPE can be set instead
	const static int MSG_NOSIGNAL = 0;
#endif //__linux__

	unsigned int messageSize = (unsigned int)message.getLength();

	//First sending package size then package content

	//Sending 0 byte packages is illegal
	if(messageSize <= 0)
	{
		messageSize = 1;
		char data = '\0';
		if(::send(fd_client, reinterpret_cast<const char*>(&messageSize), sizeof(messageSize), MSG_NOSIGNAL) != sizeof(messageSize))return false;
		if(::send(fd_client, &data, messageSize, MSG_NOSIGNAL) != int(messageSize))return false;
	}
	else
	{
		if(::send(fd_client, reinterpret_cast<const char*>(&messageSize), sizeof(messageSize), MSG_NOSIGNAL) != sizeof(messageSize))return false;
		if(::send(fd_client, message.getData(), messageSize, MSG_NOSIGNAL) != int(messageSize))return false;
	}
	return true;
}

bool IPv4CommunicationSocket::receive(Package *out)
{
	//First reading package size then package content
	unsigned int messageSize = 0;
	if(recv(fd_client, reinterpret_cast<char*>(&messageSize), sizeof(messageSize), 0) < 0)return false;
	vector<char> data(messageSize);
	if(recv(fd_client, &data[0], messageSize, 0) < 0)return false;
	if(out)out->write(&data[0], messageSize);
	return true;
}
