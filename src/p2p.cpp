#include <cluster/p2p.hpp>
#include <cluster/prototypes/address.hpp>
#include <cluster/server.hpp>
#include <cluster/client.hpp>
#include <cluster/prototypes/membercallback.hpp>
#include <vector>
#include <string>
#include <list>
#include <iostream>
#include <algorithm>
#include <unistd.h>
#include <time.h>
#include <assert.h>

using namespace std;
using namespace cluster;

const unsigned int MAX_PEER_ADDRESS_LENGTH = 50;

enum MessageType : unsigned char
{
	echo_message = 'e',
	echo_response_message = 'r',
	other_peers = 'p',
	other_peers_response = 'o'
};

p2p::p2p(const Protocol &p) :
	ClusterObject(nullptr),
	addressRangeMutex(),
	addressRanges(),
	protocol(p),
	isConnected(true),
	addresses(),
	members(),
	server(nullptr),
	testAliveThread(),
	memberMutex(),
	memberCallbacks()
{
	protocol.getAddresses(addresses);
	for(auto it = addresses.cbegin(); it != addresses.cend(); it++)
	{
		cout<<"Server address: "<<(*it)->address<<endl;
	}
	createServer();
	testAliveThread = new thread(&p2p::connectToHosts, this);
}

p2p::~p2p()
{
	for(auto it = addresses.begin(); it != addresses.end(); it++)
	{
		delete (*it);
	}
	isConnected = false;
	testAliveThread->join();
	delete testAliveThread;
	delete server;
	for(auto it = addressRanges.begin(); it != addressRanges.end(); it++)
	{
		delete it->first;
		delete it->second;
	}
}

bool p2p::isOwnAddress(const Address &a) const
{
	if(a.isLoopback())return true;
	for(auto it = addresses.begin(); it != addresses.end(); it++)
	{
		if((*(*it)) == a)return true;
	}
	return false;
}

void p2p::addAddressRange(const Address &start, const Address &end)
{
	addressRangeMutex.lock();
	addressRanges.push_back(pair<Address*,Address*>(start.clone(), end.clone()));
	addressRangeMutex.unlock();
}

void p2p::createServer()
{
	bool connected = false;
	while(!connected)
	{
		try{
			server = new Server(protocol);
			server->setCallback([this](const Address &ip, const Package &message, Package &answer)
				{
					this->received_internal(ip, message, answer);
				});
			connected = true;
		}catch(const ServerException &e){
			cout<<"ServerException: "<<e.text<<" Retrying..."<<endl;
			sleep(5);
		}
	}
}

void p2p::received_internal(const Address &ip, const Package &message, Package &answer)
{
	Package send;
	bool success = this->ClusterObject_received(ip, message, answer, send);
	if(!success)
	{
		cout<<"Invalid package: "<<message.toString()<<endl;
	}
	if(send.getLength() != 0)Client(ip,protocol).send(send);
}

void p2p::connectToHosts()
{
	time_t t1;
	time_t t2;
	time(&t1);
	while(isConnected)
	{
		if(addressRanges.size() == 0)sleep(1);

		for(unsigned int i = 0; i < addressRanges.size(); i++)
		{
			addressRangeMutex.lock();
			auto it = addressRanges.begin();
			for(unsigned int j = 0; j < i && it != addressRanges.end(); j++)it++;
			if(it == addressRanges.end())
			{
				addressRangeMutex.unlock();
				break;
			}
			Address *iterator = it->first->clone();
			Address *end = it->second->clone();
			addressRangeMutex.unlock();

			while(isConnected)
			{
				if(!isOwnAddress(*iterator))
				{
					bool online = testConnection(*iterator, 1);
					time(&t2);
					if(difftime(t2, t1) < 0.5 && !online)
					{
						cout<<"Warning: Seems like you have a network problem!"<<endl;
						sleep(1);
					}
					t1 = t2;
					if((*iterator) == (*end))break;
					iterator->increase();
				}
			}
			delete iterator;
			delete end;
		}
	}
}

bool p2p::testConnection(const Address &ip, unsigned int retry)
{
	if(members.size() == 0)cout<<"Searching "<<ip.address<<endl;
	for(unsigned int i = 0; i < retry; i++)
	{
		if(i > 0)
		{
			sleep(1);
			//cout<<"Trying connection again to "<<ip.address<<endl;
		}
		if(ask(ip, echo_message, getMembersCount()+1))return true;
	}
	offline(ip);
	return false;
}

bool p2p::ClusterObject_ask(const Address &ip, const Package &message, Package *answer)
{
	assert(message.getLength() > 0);

	return Client(ip, protocol).send(message, answer);
}

bool p2p::isMember(const Client &client)
{
	memberMutex.lock();
	bool member = find(members.begin(), members.end(), client) != members.end();
	memberMutex.unlock();
	return member;
}

bool p2p::isMember(const Address &address)
{
	memberMutex.lock();
	bool member = find(members.begin(), members.end(), address) != members.end();
	memberMutex.unlock();
	return member;
}

void p2p::online(const Address &address, unsigned int memb)
{
	//The one with more members is the master
	//If both have the same amount, the receiver is the master.
	bool isMaster = (memb > getMembersCount());

	memberMutex.lock();
	members.push_back(Client(address, protocol));
	memberMutex.unlock();

	ask(address, other_peers);

	for(auto it = memberCallbacks.begin(); it != memberCallbacks.end(); it++)
	{
		(*it)->memberOnline(address, isMaster);
	}
}

void p2p::offline(const Address &address)
{
	memberMutex.lock();
	auto index = find(members.begin(), members.end(), address);
	if(index != members.end())
	{
		for(auto it = memberCallbacks.begin(); it != memberCallbacks.end(); it++)
		{
			(*it)->memberOffline(address);
		}
		members.erase(index);
	}
	memberMutex.unlock();
}

void p2p::ClusterObject_send(const Package &message, Package *answer)
{
	list<Client> toSend;
	memberMutex.lock();
	for(auto it = members.cbegin(); it != members.cend(); it++)
	{
		toSend.push_back(*it);
	}
	memberMutex.unlock();

	while(toSend.size()+1 > (members.size()+1)/2)
	{
		for(auto it = toSend.begin(); it != toSend.end(); it++)
		{
			//Lock member mutex and check if the client is still
			//member of the cluster.
			memberMutex.lock();
			if(find(members.begin(), members.end(), (*it)) == members.end())
			{
				memberMutex.unlock();
				continue;
			}

			if(it->send(message, answer))
			{
				auto temp = it--;
				toSend.erase(temp);
			}
			memberMutex.unlock();
		}

		if(toSend.size()+1 > members.size()/2)
		{
			for(auto it = toSend.begin(); it != toSend.end(); it++)
			{
				memberMutex.lock();
				auto found = find(members.begin(), members.end(), (*it));
				memberMutex.unlock();
				if(found == members.cend())
				{
					auto temp = it--;
					toSend.erase(temp);
				}
				else testConnection(it->getAddress(), 3);	//TODO
			}
			usleep(1000);
		}
	}
}

bool p2p::received(const Address &ip, const Package &message, Package __attribute__((__unused__)) &answer, Package &send)
{
	MessageType type;
	message>>type;
	switch(type)
	{
	case echo_message:
		if(!isMember(ip))
		{
			int memb;
			message>>memb;
			online(ip, memb);
		}
		send<<echo_response_message;
		send<<members.size();
		return true;
	case echo_response_message:
		if(!isMember(ip))
		{
			int memb;
			message>>memb;
			online(ip, memb);
		}
		return true;
	case other_peers:
		send<<other_peers_response;
		memberMutex.lock();
		for(auto it = members.cbegin(); it != members.cend(); it++)
		{
			if(it->getAddress() != ip)
			{
				char address[MAX_PEER_ADDRESS_LENGTH];	//Max string length of ipv6
				const string &a = it->getAddress().address;
				copy(a.c_str(), a.c_str()+a.length(), address);
				address[a.length()] = '\0';
				send<<address;
			}
		}
		memberMutex.unlock();
		return true;
	case other_peers_response:
		char address[MAX_PEER_ADDRESS_LENGTH];
		while(message>>address)
		{
			if(Address *ip = protocol.decodeAddress(address))
			{
				testConnection(*ip, 1);
				delete ip;
			}
		}
		return true;
	default:
		return false;
	}
}

unsigned int p2p::getMembersCount() const
{
	return members.size() + 1;
}
