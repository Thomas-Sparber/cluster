#ifndef LISTENERSOCKET_HPP
#define LISTENERSOCKET_HPP

#include <string>

namespace cluster
{

class CommunicationSocket;

class ListenerException
{

public:
	ListenerException(const std::string &text) : text(text) {}
	std::string text;

}; //end class ListenerException

class ListenerSocket
{

public:
	ListenerSocket() {}
	virtual ~ListenerSocket() {}
	virtual bool poll(unsigned int time) = 0;
	virtual CommunicationSocket* listen() = 0;

}; // end class ListenerSocket

} // end namespace cluster

#endif
