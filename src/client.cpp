/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#include <cluster/client.hpp>
#include <cluster/prototypes/communicationsocket.hpp>
#include <cluster/prototypes/protocol.hpp>
#include <cluster/package.hpp>
#include <assert.h>
#include <iostream>
#include <map>
#include <mutex>
#include <thread>
#include <sys/time.h>
#include <unistd.h>

using namespace std;
using namespace cluster;

/*multimap<string, pair<CommunicationSocket*,unsigned long> > sockets;
mutex socketMutex;
thread *cleanupThread = nullptr;
mutex threadMutex;
unsigned int clientsCounter = 0;

CommunicationSocket* getSocket(const Address &address, const Protocol *protocol)
{
	CommunicationSocket *socket = nullptr;

	socketMutex.lock();
	auto it = sockets.find(address.address);
	if(it == sockets.cend())
	{
		socket = protocol->createCommunicationSocket(address);
	}
	else
	{
		socket = it->second.first;
		sockets.erase(it);
	}
	socketMutex.unlock();

	return socket;
}

void cleanupThreadFunction()
{
	while(clientsCounter != 0)
	{
		struct timeval time;
		gettimeofday(&time, nullptr);

		socketMutex.lock();
		for(auto it = sockets.begin(); it != sockets.end(); ++it)
		{
			const unsigned long diff = time.tv_sec - it->second.second;

			if(diff >= 10)
			{
				cout<<"Closing connection to "<<it->first<<endl;
				delete it->second.first;
				sockets.erase(it);
				break;
			}
		}
		socketMutex.unlock();

		sleep(1);
	}

	socketMutex.lock();
	for(auto it = sockets.begin(); it != sockets.end(); ++it)
	{
		delete it->second.first;
	}
	socketMutex.unlock();
	sockets.clear();
}

void putSocket(const Address &address, CommunicationSocket *socket)
{
	struct timeval time;
	gettimeofday(&time, nullptr);

	socketMutex.lock();
	sockets.insert(pair<string,pair<CommunicationSocket*,unsigned long> >(address.address, pair<CommunicationSocket*,unsigned long>(socket, time.tv_sec)));
	socketMutex.unlock();

	threadMutex.lock();
	if(!cleanupThread)
	{
		cleanupThread = new thread(cleanupThreadFunction);
	}
	threadMutex.unlock();
}*/

/***********************************************/

Client::Client(const Address &a, const Protocol &p) :
	address(a.clone()),
	protocol(&p)
{
/*	threadMutex.lock();
	++clientsCounter;
	threadMutex.unlock();*/
}

Client::Client(const Client &c) :
	address(c.address->clone()),
	protocol(c.protocol)
{
/*	threadMutex.lock();
	++clientsCounter;
	threadMutex.unlock();*/
}

Client& Client::operator=(const Client &c)
{
	assert(c.address != nullptr);

	protocol = c.protocol;
	delete address;
	address = c.address->clone();
	return (*this);
}

Client::~Client()
{
	delete address;

/*	threadMutex.lock();
	if(--clientsCounter == 0 && cleanupThread)
	{
		cleanupThread->join();
		delete cleanupThread;
		cleanupThread = nullptr;
	}
	threadMutex.unlock();*/
}

bool Client::send(const Package &message, Package *out) const
{
	//Create communication socket
	if(CommunicationSocket *s = protocol->createCommunicationSocket(*address))
	{
		bool success = s->send(message) && s->receive(out);
/*cout<<message.toString()<<(success ? " success" : " no success")<<endl;
		if(!success)
		{
			//Try again using a new connection*/
			delete s;

/*			s = getSocket(*address, protocol);
			success = s->send(message) && s->receive(out);
cout<<(success ? " success" : " no success")<<endl;
		}

		putSocket(*address, s);*/
		return success;
	}
	return false;
}

bool Client::operator==(const Client &c)
{
	return (*address) == (*c.address);
}

bool Client::operator==(const Address &a)
{
	return (*address) == a;
}
