/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#include <cluster/prototypes/protocol.hpp>
#include <cluster/server.hpp>
#include <cluster/package.hpp>
#include <cluster/prototypes/listenersocket.hpp>
#include <cluster/prototypes/communicationsocket.hpp>
#include <iostream>
#include <unistd.h>

using namespace std;
using namespace cluster;

Server::Server(const Protocol &p_protocol) :
	running(true),
	socket(nullptr),
	protocol(&p_protocol),
	callback(nullptr),
	t(),
	answerThread(),
	requests(),
	m()
{
	//Open listener connection
	openConnection();

	//Start thread that polls and accepts connection
	t = new thread(&Server::serverFunction, this);

	//Start the threads which handle the connections
	for(unsigned int i = 0; i < handlersCount; i++)
	{
		answerThread[i] = new thread(&Server::handle, this);
	}
}

void Server::openConnection()
{
	if(!(socket = protocol->createListenerSocket()))throw ServerException("Could not open Listener socket");
}

Server::~Server()
{
	running = false;
	t->join();
	delete t;
	for(unsigned int i = 0; i < handlersCount; i++)
	{
		answerThread[i]->join();
		delete answerThread[i];
	}
	while(requests.size())
	{
		delete requests.front();
		requests.pop();
	}
	closeConnection();
}

void Server::closeConnection()
{
	cout<<"Closing server socket"<<endl;
	delete socket;
}

void Server::serverFunction()
{
	while(running)
	{
		if(!socket->poll(2000))continue;

		//Accept connections from client
		if(CommunicationSocket *client = socket->listen())
		{
			m.lock();
			requests.push(client);
			m.unlock();
		}
		else
		{
			//Reopen connection
			do {
				try {
					closeConnection();
					openConnection();
				} catch(const ServerException &e) {
					sleep(1);
				}
			}
			while(running && !socket);
		}
	}
	cout<<"Server closed"<<endl;
}

void Server::handle()
{
	while(running)
	{
		m.lock();
		if(requests.empty())
		{
			m.unlock();
			usleep(1000);
			continue;
		}
		CommunicationSocket *client = requests.front();
		requests.pop();
		m.unlock();

		//read data from client
		Package p;
		Package answer;
		client->receive(&p);

		//Call callback and get answer
		if(callback)callback(client->getAddress(), p, answer);

		client->send(answer);
		delete client;
	}
}
