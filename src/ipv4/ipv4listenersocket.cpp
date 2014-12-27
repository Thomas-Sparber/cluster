#include <cluster/ipv4/ipv4listenersocket.hpp>
#include <cluster/ipv4/ipv4communicationsocket.hpp>
#include <cluster/ipv4/ipv4address.hpp>
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

const unsigned int LISTEN_BACKLOG = 50;

IPv4ListenerSocket::IPv4ListenerSocket(uint16_t port, unsigned int timeout) :
	ListenerSocket(),
	port(port),
	fd_socket(),
	timeout(timeout)
{
	struct sockaddr_in addr4;
	fd_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(fd_socket == -1)throw ListenerException("Unable to create socket.");
	addr4.sin_family = AF_INET;
	addr4.sin_port = htons(port);
	addr4.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(fd_socket, reinterpret_cast<const sockaddr*>(&addr4), sizeof(addr4)) == -1)
	{
		close(fd_socket);
		throw ListenerException("Unable to bind server socket.");
	}
	if(::listen(fd_socket, LISTEN_BACKLOG) == -1)
	{
		close(fd_socket);
		throw ListenerException("Unable to listen on server socket.");
	}
}

IPv4ListenerSocket::~IPv4ListenerSocket()
{
	close(fd_socket);
}

bool IPv4ListenerSocket::poll(unsigned int time)
{
	struct pollfd fd;
	fd.fd = fd_socket;
	fd.events = POLLIN;
	return ::poll(&fd, 1, time);
}

CommunicationSocket* IPv4ListenerSocket::listen()
{
	struct sockaddr_in clientAddr;
	unsigned int size = sizeof(struct sockaddr_in);
	int fd_client = accept(fd_socket, reinterpret_cast<struct sockaddr*>(&clientAddr), &size);
	if(fd_client == -1)
	{
		//Probably too many open connections
		//cout<<"Could not accept client connection"<<endl;
		//cout<<strerror(errno)<<endl;
		return nullptr;
	}
	char addressBuffer[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &clientAddr.sin_addr.s_addr, addressBuffer, INET_ADDRSTRLEN);
	IPv4Address ipAddress(addressBuffer);
	return new IPv4CommunicationSocket(ipAddress, port, fd_client, timeout);
}
