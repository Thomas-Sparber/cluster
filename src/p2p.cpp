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
#include <sys/time.h>

using namespace std;
using namespace cluster;

const unsigned int MAX_PEER_ADDRESS_LENGTH = 50;

const unsigned char echo_message = 'e';
const unsigned char echo_response_message = 'r';
const unsigned char other_peers = 'p';
const unsigned char other_peers_response = 'o';

p2p::p2p(const Protocol &p) :
	ClusterObject(nullptr),
	addressRangeMutex(),
	addressRanges(),
	protocol(p),
	isConnected(true),
	otherPeersChecked(false),
	addresses(),
	members(),
	server(nullptr),
	testAliveThread(nullptr),
	memberMutex(),
	memberCallbacks(),
	startTime()
{
	struct timeval t;
	gettimeofday(&t, nullptr);
	startTime = (unsigned long long)t.tv_sec * 1000000 + t.tv_usec;

	protocol.getAddresses(addresses);
	for(auto it = addresses.cbegin(); it != addresses.cend(); it++)
	{
		cout<<"Server address: "<<(*it)->address<<endl;
	}
	open();
}

p2p::~p2p()
{
	close();

	for(auto it = addresses.begin(); it != addresses.end(); it++)
	{
		delete (*it);
	}
	for(auto it = addressRanges.begin(); it != addressRanges.end(); it++)
	{
		delete it->first;
		delete it->second;
	}
}

void p2p::open()
{
	if(!server)
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

	if(!testAliveThread)
	{
		testAliveThread = new thread(&p2p::connectToHosts, this);
	}
}

void p2p::close()
{
	isConnected = false;

	if(testAliveThread)
	{
		testAliveThread->join();
		delete testAliveThread;
		testAliveThread = nullptr;
	}

	if(server)
	{
		delete server;
		server = nullptr;
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

void p2p::received_internal(const Address &ip, const Package &message, Package &answer)
{
	Package to_send;
	bool success = this->ClusterObject_received(ip, message, answer, to_send);
	if(!success)
	{
		cout<<"Invalid package: "<<message.toString()<<endl;
	}
	if(to_send.getLength() != 0)Client(ip,protocol).send(to_send);
}

void p2p::connectToHosts()
{
	time_t t1;
	time_t t2;
	time(&t1);

	auto it = addressRanges.cbegin();
	Address *currentAddress = nullptr;
	Address *currentAddressEnd = nullptr;
	while(isConnected)
	{
		//Continue if no address ranges exist
		if(it == addressRanges.cend())
		{
			sleep(1);
			continue;
		}

		//Create address ranges from iterator
		if(!currentAddress || !currentAddressEnd)
		{
			currentAddress = it->first->clone();
			currentAddressEnd = it->second->clone();
		}

		//Increase iterator if address range end reached
		if((*currentAddress) == (*currentAddressEnd))
		{
			delete currentAddress;
			delete currentAddressEnd;
			currentAddress = nullptr;
			currentAddressEnd = nullptr;

			addressRangeMutex.lock();
			if(++it == addressRanges.cend())
			{
				it = addressRanges.cbegin();
			}
			addressRangeMutex.unlock();
			continue;	//Continuing to create address ranges from iterator
		}
		
		
		if(!isOwnAddress(*currentAddress))
		{
			bool is_online = testConnection(*currentAddress, 1);
			time(&t2);
			if(difftime(t2, t1) < 0.2 && !is_online)sleep(1);
			t1 = t2;
		}

		currentAddress->increase();
	}

	delete currentAddress;
	delete currentAddressEnd;
}

bool p2p::testConnection(const Address &ip, unsigned int retry)
{
	if(members.empty())
	{
		cout<<"Trying connection to "<<ip.address<<endl;
	}

	for(unsigned int i = 0; i < retry; i++)
	{
		if(i > 0)
		{
			cout<<"Retrying connection to "<<ip.address<<endl;
			sleep(1);
		}

		if(ask(ip, echo_message, startTime))return true;
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

void p2p::online(const Address &address, unsigned long long otherTime)
{
	//The one with the smaller startTime is the master
	bool isMaster = (otherTime < startTime);

	cout<<"Online "<<address.address<<": "<<otherTime<<", "<<startTime<<endl;
	otherPeersChecked = false;

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
	if(members.size() == 0)otherPeersChecked = false;
	memberMutex.unlock();
}

bool p2p::ClusterObject_send(const Package &message, Package *answer)
{
	while(!isReady())usleep(1000);

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

	return true;
}

bool p2p::received(const Address &ip, const Package &message, Package &/*answer*/, Package &to_send)
{
	unsigned char type;
	message>>type;
	switch(type)
	{
	case echo_message:
	{
		if(!isMember(ip))
		{
			unsigned long long otherTime;
			message>>otherTime;
			online(ip, otherTime);
			cout<<"Online "<<ip.address<<endl;
		}
		to_send<<echo_response_message;
		to_send<<startTime;
		return true;
	}
	case echo_response_message:
	{
		if(!isMember(ip))
		{
			unsigned long long otherTime;
			message>>otherTime;
			online(ip, otherTime);
			cout<<"Online from response "<<ip.address<<endl;
		}
		return true;
	}
	case other_peers:
		to_send<<other_peers_response;
		memberMutex.lock();
		for(auto it = members.cbegin(); it != members.cend(); it++)
		{
			if(it->getAddress() != ip)
			{
				char address[MAX_PEER_ADDRESS_LENGTH];	//Max string length of ipv6
				const string &a = it->getAddress().address;
				copy(a.c_str(), a.c_str()+a.length(), address);
				address[a.length()] = '\0';
				to_send<<address;
			}
		}
		memberMutex.unlock();
		return true;
	case other_peers_response: {
		char address[MAX_PEER_ADDRESS_LENGTH];
		list<Address*> tested;
		while(message>>address)
		{
			if(Address *a = protocol.decodeAddress(address))
			{
				if(testConnection(*a, 1))tested.push_back(a);
				else delete a;
			}
		}
		for(unsigned int i = 0; i < 100; ++i)
		{
			bool allAnswered = true;
			for(Address *a : tested)
			{
				if(!isMember(*a))
				{
					allAnswered = false;
					break;
				}
			}
			if(allAnswered)break;
			usleep(1000);
		}
		for(Address *a : tested)delete a;
		otherPeersChecked = true;
		return true;
	}
	default:
		return false;
	}
}

unsigned int p2p::getMembersCount() const
{
	return members.size();
}
