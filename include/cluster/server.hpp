#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <thread>
#include <queue>
#include <mutex>

namespace cluster
{

class Protocol;
class Package;
class ListenerSocket;
class CommunicationSocket;

class ServerException
{

public:
	ServerException(const std::string &text) : text(text) {}
	std::string text;

}; // end class ServerException

class Server
{

public:
	Server(const Protocol &protocol);
	Server(const Server &s);
	Server& operator=(const Server &s);
	~Server();
	void setCallback(std::function<void(const Address &from, const Package &data, Package &answer)> callback)
	{
		this->callback = callback;
	}

private:
	void openConnection();
	void closeConnection();
	void serverFunction();
	void handle();

private:
	static const unsigned int handlersCount = 20;

	bool running;
	//int fd_socket;
	ListenerSocket *socket;
	const Protocol *protocol;
	std::function<void(const Address &from, const Package &data, Package &answer)> callback;
	std::thread *t;
	std::thread *answerThread[handlersCount];
	std::queue<CommunicationSocket*> requests;
	std::mutex m;

};

} //end namespacee cluster

#endif
