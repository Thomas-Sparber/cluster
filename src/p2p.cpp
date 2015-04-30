/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

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

namespace cluster
{

	/**
	  * This enum defines the messages
	  * which are used to communicate between
	  * p2p networks
	 **/
	enum class p2pOperation : char
	{

		/**
		  * An echo message is sent to computers
		  * to check if they respond
		 **/
		echo_message = 'e',

		/**
		  * An echo response message is answered to
		  * an echo message to signal that the client
		  * wants to connect
		 **/
		echo_response_message = 'r',

		/**
		  * This message means that the target client
		  * should send its other peers
		 **/
		other_peers = 'p',

		/**
		  * This is the response to the other peers
		  * message which also includes the addresses
		  * of the other peers
		 **/
		other_peers_response = 'o',

		/**
		  * This messages means that the sender went offline
		 **/
		went_offline = 'w'

	}; //end enum p2pOperation

	/**
	  * This function is overloaded from the Package class
	  * to retrieve a p2pOperation from a Package
	 **/
	template <>
	inline bool operator>>(const Package &p, p2pOperation &t)
	{
		return p>>reinterpret_cast<char&>(t);
	}


	/**
	  * This function is overloaded from the Package class
	  * to insert a p2pOperation into a Package
	 **/
	template <>
	inline void operator<<(Package &p, const p2pOperation &t)
	{
		p<<reinterpret_cast<const char&>(t);
	}

} //end namespace cluster

p2p::p2p(const Protocol &p) :
	ClusterObject(nullptr),
	addressRangeMutex(),
	addressRanges(),
	protocol(p),
	isConnected(true),
	otherPeersToCheck(),
	otherPeersToCheckMutex(),
	addresses(),
	members(),
	server(nullptr),
	testAliveThread(nullptr),
	memberMutex(),
	memberCallbacks(),
	startTime(),
	reconnectRetries(3),
	continueWithoutMembers(true),
	callbackMutex()
{
	//Measure start time to determine master host
	struct timeval t;
	gettimeofday(&t, nullptr);
	startTime = (unsigned long long)t.tv_sec * 1000000 + t.tv_usec;

	//Get Addresses of current computer
	protocol.getAddresses(addresses);
	for(auto it = addresses.cbegin(); it != addresses.cend(); it++)
	{
		cout<<"Server address: "<<(*it)->address<<endl;
	}

	//Open server socket and search thread
	open();
}

p2p::~p2p()
{
	//Tell others that I'm going offline
	send(p2pOperation::went_offline, nullptr);

	//Close server socket and seach thread
	close();

	//Delete own addresses
	for(auto it = addresses.begin(); it != addresses.end(); it++)
	{
		delete (*it);
	}

	//Delete address ranges to search
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
		//Open Server socket
		bool connected = false;
		while(!connected)
		{
			try{
				server = new Server(protocol);

				//Set callback
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
	if(!to_send.empty())Client(ip,protocol).send(to_send);
}

void p2p::connectToHosts()
{
	time_t t1;
	time_t t2;
	time(&t1);

	unsigned int memberIndex = 0;
	bool askMember = true;

	auto it = addressRanges.cbegin();
	Address *currentAddress = nullptr;
	Address *currentAddressEnd = nullptr;
	while(isConnected)
	{
		//Members are checked every second time
		if((askMember = !askMember))
		{
			Address *a = nullptr;

			memberMutex.lock();
			if(memberIndex >= members.size())memberIndex = 0;
			if(memberIndex < members.size())
			{
				auto memberIt = members.cbegin();
				for(unsigned int i = 0; i < memberIndex; ++i, ++memberIt);
				a = memberIt->getAddress().clone();
			}
			memberMutex.unlock();

			if(a)testConnection(*a, 1);
			delete a;
		}

		//Continue if no address ranges exist
		if(it == addressRanges.cend())
		{
			sleep(1);
			it = addressRanges.cbegin();
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
	/*if(members.empty())
	{
		cout<<"Trying connection to "<<ip.address<<endl;
	}*/

	for(unsigned int i = 0; i < retry; i++)
	{
		if(i > 0)
		{
			cout<<"Retrying connection to "<<ip.address<<endl;
			sleep(1);
		}

		if(ask(ip, p2pOperation::echo_message, startTime, nullptr))return true;
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

	const Client client(address, protocol);

//	cout<<"Online "<<address.address<<": "<<otherTime<<", "<<startTime<<endl;

	//Remember host that it needs to answer its members
	otherPeersToCheckMutex.lock();
	otherPeersToCheck.push_back(client);
	otherPeersToCheckMutex.unlock();

	//Add to members
	memberMutex.lock();
	members.push_back(client);
	memberMutex.unlock();

	//Ask for other peers
	ask(address, p2pOperation::other_peers, nullptr);

	//Notify callbacks
	callbackMutex.lock();
	for(auto it = memberCallbacks.begin(); it != memberCallbacks.end(); it++)
	{
		(*it)->memberOnline(address, isMaster);
	}
	callbackMutex.unlock();
}

void p2p::offline(const Address &address)
{
	bool wasMember = false;

	memberMutex.lock();
	auto index = find(members.begin(), members.end(), address);
	if(index != members.end())
	{
		wasMember = true;
		members.erase(index);
	}
	memberMutex.unlock();

	//Notify callbacks
	if(wasMember)
	{
		callbackMutex.lock();
		for(auto it = memberCallbacks.begin(); it != memberCallbacks.end(); it++)
		{
			(*it)->memberOffline(address);
		}
		callbackMutex.unlock();
	}
}

bool p2p::ClusterObject_send(const Package &message, AnswerPackage *answer)
{
	//Other peers need to be checked before sending
	otherPeersToCheckMutex.lock();
	for(unsigned int i = 0; i < 50000 && !otherPeersToCheck.empty(); ++i)
	{
		otherPeersToCheckMutex.unlock();
		usleep(1000);
		otherPeersToCheckMutex.lock();
	}

	if(!otherPeersToCheck.empty())
	{
		cout<<"Didn't get p2pOperation::other_peers_response from "<<otherPeersToCheck.size()<<" client(s)"<<endl;
		memberMutex.lock();
		for(const Client &c : otherPeersToCheck)
		{
			auto it = find(members.begin(), members.end(), c);
			if(it == members.end())cout<<"Strange: Waited for p2pOperation::other_peers_response for a client who isn't a memeber"<<endl;
			else members.erase(it);
		}
		memberMutex.unlock();
		otherPeersToCheck.clear();
	}
	otherPeersToCheckMutex.unlock();

	//Sending Package to every member
	list<Client> toSend;
	memberMutex.lock();
	for(auto it = members.cbegin(); it != members.cend(); it++)
	{
		toSend.push_back(*it);
	}
	memberMutex.unlock();

	//At least one more than half of the members need to respond
	while((!continueWithoutMembers || !toSend.empty()) && toSend.size()+1 > (members.size()+1)/2)
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

			bool success;
			if(answer)
			{
				Package temp_answer;
				success = it->send(message, &temp_answer);
				if(success)answer->add(it->getAddress(), temp_answer);
			}
			else
			{
				success = it->send(message, nullptr);
			}

			if(success)
			{
				auto temp = it--;
				toSend.erase(temp);
			}
			memberMutex.unlock();
		}

		//Test connection if not enough members answered
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
				else testConnection(it->getAddress(), reconnectRetries);
			}

			//Wait for some time to let clients respond
			usleep(1000);
		}
	}

	return true;
}

bool p2p::received(const Address &ip, const Package &message, Package &/*answer*/, Package &to_send)
{
	p2pOperation type;
	if(!(message>>type))return false;
	switch(type)
	{
	case p2pOperation::echo_message:
		if(!isMember(ip))
		{
			//Save client as memeber
			unsigned long long otherTime;
			if(!(message>>otherTime))return false;
			online(ip, otherTime);
		}
		to_send<<p2pOperation::echo_response_message;
		to_send<<startTime;
		return true;
	case p2pOperation::echo_response_message:
		if(!isMember(ip))
		{
			//Save client as memeber
			unsigned long long otherTime;
			if(!(message>>otherTime))return false;
			online(ip, otherTime);
		}
		return true;
	case p2pOperation::other_peers:
		//Send current memebers
		to_send<<p2pOperation::other_peers_response;
		memberMutex.lock();
		for(auto it = members.cbegin(); it != members.cend(); it++)
		{
			if(it->getAddress() != ip)
			{
				const string &address = it->getAddress().address;
				to_send<<address;
			}
		}
		memberMutex.unlock();
		return true;
	case p2pOperation::other_peers_response: {
		//Extract other peers from message
		string address;
		list<Address*> tested;
		while(message>>address)
		{
			if(Address *a = protocol.decodeAddress(address))
			{
				if(!isOwnAddress(*a) && !isMember(*a) && testConnection(*a, 1))tested.push_back(a);
				else delete a;
			}
		}

		//Wait for all peers to answer
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

		otherPeersToCheckMutex.lock();
		auto it = find(otherPeersToCheck.begin(), otherPeersToCheck.end(), Client(ip, protocol));
		if(it == otherPeersToCheck.end())cout<<"Strange: Got p2pOperation::other_peers_response from a client I didn't ask."<<endl;
		else otherPeersToCheck.erase(it);
		otherPeersToCheckMutex.unlock();
		return true;
	}
	case p2pOperation::went_offline:
		offline(ip);
		return true;
	default:
		return false;
	}
}
