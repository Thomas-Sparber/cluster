/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace cluster
{

class Protocol;
class Package;
class ListenerSocket;
class CommunicationSocket;

/**
  * This exception could be thrown from the constructor
  * when it was unable to bind the server
 **/
class ServerException
{

public:
	/**
	  * Constructs a server exception with a description text
	 **/
	ServerException(const std::string &str_text) : text(str_text) {}

	/**
	  * The error description
	 **/
	std::string text;

}; // end class ServerException

/**
  * This class is responsible to listen for connections
  * and accept and handle them.
 **/
class Server
{

public:
	/**
	  * Constructs a Server using the given protocol.
	  * Throws ServerException if the listener can not be bound
	 **/
	Server(const Protocol &protocol);

	/**
	  * Default destructor
	 **/
	~Server();

	/**
	  * Registers the given function as callback
	  * which is called whenever a new Package is received
	 **/
	void setCallback(std::function<void(const Address &from, const Package &data, Package &answer)> fn_callback)
	{
		this->callback = fn_callback;
	}

private:
	/**
	  * Copying a Server is illegal
	 **/
	Server(const Server &s);

	/**
	  * Copying a Server is illegal
	 **/
	Server& operator=(const Server &s);

	/**
	  * Opens the listening connection
	 **/
	void openConnection();

	/**
	  * Closes the listening connection
	 **/
	void closeConnection();

	/**
	  * This function continually listens and accepts
	  * connections and adds them to the queue to be handled
	 **/
	void serverFunction();

	/**
	  * Handles the next connection in the queue
	 **/
	void handle();

private:
	/**
	  * Determines the amount of threads to handle
	  * connections. This is equivalent to the number
	  * of maximum concurrent connections
	 **/
	static const unsigned int handlersCount = 20;

	/**
	  * A flag which is used to manage the running status
	 **/
	bool running;

	/**
	  * The listener socket which is used to listen
	  * for connections.
	 **/
	ListenerSocket *socket;

	/**
	  * The protocol which is used to create the
	  * ListenerSocket and CommunicationSocket
	 **/
	const Protocol *protocol;

	/**
	  * The callback function which is called
	  * for every Package
	 **/
	std::function<void(const Address &from, const Package &data, Package &answer)> callback;

	/**
	  * The listener thread
	 **/
	std::thread *t;

	/**
	  * The threads which handle the connections
	 **/
	std::thread *answerThread[handlersCount];

	/**
	  * The communication sockets which need to be handled
	 **/
	std::queue<CommunicationSocket*> requests;

	/**
	  * Allows paralled access to the requests queue
	 **/
	std::mutex m;

	/**
	  * The condition variable which is used to notify the threads
	  * to accept connections
	 **/
	std::condition_variable cv;

	/**
	  * The mutext which is needed for the threads to wait.
	 **/
	std::mutex cm;

};

} //end namespacee cluster

#endif
