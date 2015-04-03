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
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <poll.h>
#include <string.h>
#include <unistd.h>

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
	if(fd_socket == -1)throw ListenerException("Unable to create socket.");

	//Reuse address
	static const int yes = 1;
	setsockopt(fd_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

	//Open port
	struct sockaddr_in6 addr6;
	addr6.sin6_family = AF_INET6;
	addr6.sin6_port = htons(port);
	addr6.sin6_addr = in6addr_any;

	//Bind socket
	if(bind(fd_socket, reinterpret_cast<const sockaddr*>(&addr6), sizeof(addr6)) == -1)
	{
		close(fd_socket);
		throw ListenerException("Unable to bind server socket.");
	}

	//Open listener
	if(::listen(fd_socket, listenBacklog) == -1)
	{
		close(fd_socket);
		throw ListenerException("Unable to listen on server socket.");
	}
}

IPv6ListenerSocket::~IPv6ListenerSocket()
{
	close(fd_socket);
}

bool IPv6ListenerSocket::poll(unsigned int time)
{
	struct pollfd fd;
	fd.fd = fd_socket;
	fd.events = POLLIN;
	return ::poll(&fd, 1, time);
}

CommunicationSocket* IPv6ListenerSocket::listen()
{
	struct sockaddr_in6 clientAddr;
	unsigned int size = sizeof(struct sockaddr_in6);
	int fd_client = accept(fd_socket, reinterpret_cast<struct sockaddr*>(&clientAddr), &size);
	if(fd_client == -1)
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
