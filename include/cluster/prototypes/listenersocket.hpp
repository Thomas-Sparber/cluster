/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef LISTENERSOCKET_HPP
#define LISTENERSOCKET_HPP

#include <string>

namespace cluster
{

class CommunicationSocket;

/**
  * This exception is thrown whenever a
  * ListenerSocket can not be created
 **/
class ListenerException
{

public:
	/**
	  * Constructs a ListenerException with a
	  * description text.
	 **/
	ListenerException(const std::string &str_text) : text(str_text) {}

	/**
	  * The description of the eexception
	 **/
	std::string text;

}; //end class ListenerException

/**
  * This class can be used to listen for
  * client connections.
 **/
class ListenerSocket
{

public:
	/**
	  * Default constructor
	 **/
	ListenerSocket() {}

	/**
	  * Default destructor
	 **/
	virtual ~ListenerSocket() {}

	/**
	  * Cheks for the given timesout if a
	  * client is waiting for a connection.
	 **/
	virtual bool poll(unsigned int time) = 0;

	/**
	  * Accepts the client connection. Doesn't
	  * return until the connection is established
	 **/
	virtual CommunicationSocket* listen() = 0;

}; // end class ListenerSocket

} // end namespace cluster

#endif //LISTENERSOCKET_HPP
