/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#include <cluster/ipv6/ipv6listenersocket.hpp>
#include <cluster/ipv6/ipv6communicationsocket.hpp>
#include <cluster/ipv6/ipv6address.hpp>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <iostream>

#ifdef __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <poll.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cluster/prototypes/windowshelpers.hpp>
#endif //__linux__

using namespace std;
using namespace cluster;

IPv6ListenerSocket::IPv6ListenerSocket(uint16_t ui_port, unsigned int ui_timeout, unsigned int listenBacklog) :
	ListenerSocket(),
	port(ui_port),
	fd_socket(),
	timeout(ui_timeout)
{
	//Open socket
	fd_socket = socket(AF_INET6, SOCK_STREAM, 0);
#ifdef __linux__
	if(fd_socket == -1)
		throw ListenerException(string("Unable to create socket: ")+strerror(errno));
#else
	if(fd_socket == INVALID_SOCKET)
		throw ListenerException(string("Unable to create socket: ")+to_string(WSAGetLastError()));
#endif //__linux__

	//Reuse address
	static const int yes = 1;
	setsockopt(fd_socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&yes), sizeof(int));

	//Open port
	struct sockaddr_in6 addr6;
	addr6.sin6_family = AF_INET6;
	addr6.sin6_port = uint16_t(htons(port));
	addr6.sin6_addr = in6addr_any;

	//Bind socket
	if(bind(fd_socket, reinterpret_cast<const sockaddr*>(&addr6), sizeof(addr6)) != 0)
	{
#ifdef __linux__
		close(fd_socket);
		throw ListenerException(string("Unable to bind server socket: ")+strerror(errno));
#else
		closesocket(fd_socket);
		throw ListenerException(string("Unable to bind server socket: ")+to_string(WSAGetLastError()));
#endif //__linux__
	}

	//Open listener
	if(::listen(fd_socket, listenBacklog) == -1)
	{
#ifdef __linux__
		close(fd_socket);
		throw ListenerException(string("Unable to listen on server socket: ")+strerror(errno));
#else
		closesocket(fd_socket);
		throw ListenerException(string("Unable to listen on server socket: ")+to_string(WSAGetLastError()));
#endif //__linux__
	}
}

IPv6ListenerSocket::~IPv6ListenerSocket()
{
#ifdef __linux__
	close(fd_socket);
#else
	closesocket(fd_socket);
#endif //__linux__
}

bool IPv6ListenerSocket::poll(unsigned int time)
{
#ifdef __linux__
	struct pollfd fd;
	fd.fd = fd_socket;
	fd.events = POLLIN;
	const int result = ::poll(&fd, 1, time);
#else
	fd_set fds;
	struct timeval to;

	to.tv_sec = time;
	to.tv_usec = 0;

	FD_ZERO(&fds);
	FD_SET(fd_socket, &fds);

	const int result = select(1, &fds, nullptr, nullptr, &to);
#endif //__linux__

	return result > 0;
}

CommunicationSocket* IPv6ListenerSocket::listen()
{
	struct sockaddr_in6 clientAddr;
#ifdef __linux__
	socklen_t size = sizeof(struct sockaddr_in);
	int fd_client = accept(fd_socket, reinterpret_cast<struct sockaddr*>(&clientAddr), &size);
	if(fd_client == -1)
#else
	int size = sizeof(struct sockaddr_in);
	SOCKET fd_client = accept(fd_socket, reinterpret_cast<struct sockaddr*>(&clientAddr), &size);
	if(fd_client == INVALID_SOCKET)
#endif //__linux__
	{
		//Probably too many open connections
		//cout<<"Could not accept client connection"<<endl;
		//cout<<strerror(errno)<<endl;
		return nullptr;
	}
	char addressBuffer[INET6_ADDRSTRLEN];
	inet_ntop(AF_INET6, &clientAddr.sin6_addr, addressBuffer, INET6_ADDRSTRLEN);
	IPv6Address ipAddress(addressBuffer);
	return new IPv6CommunicationSocket(ipAddress, port, fd_client, timeout);
}
