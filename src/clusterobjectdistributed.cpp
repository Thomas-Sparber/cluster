/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#include <cluster/clusterobjectdistributed.hpp>
#include <iostream>
#include <set>
#include <thread>
#include <unistd.h>

using namespace std;
using namespace cluster;

namespace cluster
{

/**
  * This enum defines the actions of the
  * ClusterObjectDistributed
 **/
enum class ClusterObjectDistributedOperation : char
{
	/**
	  * Defines that the package is a command package
	 **/
	command = 'c',

	/**
	  * Defines that the package is intended for ClusterObjectDistributed
	 **/
	own = 'o'

};

/**
  * This function is overloaded from the Package class
  * to retrieve a ClusterObjectDistributedOperation from a Package
 **/
template <>
inline bool operator>>(const Package &p, ClusterObjectDistributedOperation &t)
{
	return p>>reinterpret_cast<char&>(t);
}


/**
  * This function is overloaded from the Package class
  * to insert a ClusterObjectDistributedOperation into a Package
 **/
template <>
inline void operator<<(Package &p, const ClusterObjectDistributedOperation &t)
{
	p<<reinterpret_cast<const char&>(t);
}

/**
  * This enum defines the own actions of the
  * ClusterObjectDistributed
 **/
enum class OwnOperation : char
{
	/**
	  * Defines that the package is an inserted package
	 **/
	inserted = 'i',

	/**
	  * Defines that the package is an insert package
	 **/
	insert = 'j',

	/**
	  * Defines that the package is a fetch package
	 **/
	fetch = 'f',

	/**
	  * Defines that the package is a delete package
	 **/
	deleted = 'd',

	/**
	  * Indicates to send all ids
	 **/
	all_ids = 'a'
};

/**
  * This function is overloaded from the Package class
  * to retrieve a OwnOperation from a Package
 **/
template <>
inline bool operator>>(const Package &p, OwnOperation &t)
{
	return p>>reinterpret_cast<char&>(t);
}


/**
  * This function is overloaded from the Package class
  * to insert a OwnOperation into a Package
 **/
template <>
inline void operator<<(Package &p, const OwnOperation &t)
{
	p<<reinterpret_cast<const char&>(t);
}

} //end namespace cluster

ClusterObjectDistributed::ClusterObjectDistributed(ClusterObject *network, unsigned int ui_takeOverSize, unsigned int ui_dataRedundancy, bool b_lockOnInsert, unsigned int ui_maxPackagesToRemember) :
	ClusterObjectSerialized(network, ui_maxPackagesToRemember),
	dataRedundancy(ui_dataRedundancy),
	takeOverSize(ui_takeOverSize),
	lockOnInsert(b_lockOnInsert),
	onlineClients(),
	idsInClients(),
	onlineClientsMutex(),
	insertMutex(network)
{
	//Add local client
	onlineClients.push_back(ClientRecord(true));

	srand((unsigned int)time(nullptr));
}

ClusterObjectDistributed::~ClusterObjectDistributed()
{}

std::size_t ClusterObjectDistributed::getOnlineClientId(const Address &ip) const
{
	std::size_t index = 0xFFFFFFFF;

	onlineClientsMutex.lock();
	for(std::size_t i = 0; i < onlineClients.size(); ++i)
	{
		if(onlineClients[i] == ip)
		{
			index = i;
			break;
		}
	}
	onlineClientsMutex.unlock();

	return index;
}

void ClusterObjectDistributed::addIdsToLocalClient(const list<string> &ids)
{
	//Inform other clients
	Package pkg;
	pkg<<ClusterObjectDistributedOperation::own;
	pkg<<OwnOperation::inserted;
	for(const string id : ids)
	{
		onlineClients[0].ids.push_back(id);
		idsInClients[id].push_back(0);
		pkg<<id;
//		cout<<"Adding "<<id<<endl;
	}
	while(ClusterObjectSerialized::sendPackage(pkg, nullptr))usleep(1000);
}

void ClusterObjectDistributed::setInitialIds(const list<string> &ids)
{
	//Inform other clients
	Package pkg;
	pkg<<ClusterObjectDistributedOperation::own;
	pkg<<OwnOperation::inserted;
	for(const string id : ids)
	{
		onlineClients[0].ids.push_back(id);
		idsInClients[id].push_back(0);
		pkg<<id;
//		cout<<"Adding "<<id<<endl;
	}
	while(!ClusterObjectSerialized::sendPackage(pkg, nullptr))usleep(1000);
}

void ClusterObjectDistributed::memberOnline(const Address &ip, bool isMaster)
{
	ClusterObjectSerialized::memberOnline(ip, isMaster);

	onlineClientsMutex.lock();
	onlineClients.push_back(ClientRecord(ip));

	//Ask member for ids
	Package message;
	message<<ClusterObjectDistributedOperation::own;
	message<<OwnOperation::all_ids;
	Package answer;
	ClusterObjectSerialized::askPackage(ip, message, &answer);
	string id;
	while(answer>>id)
	{
		onlineClients[onlineClients.size()-1].ids.push_back(id);
		idsInClients[id].push_back(onlineClients.size()-1);
	}

	onlineClientsMutex.unlock();
}

void ClusterObjectDistributed::memberOffline(const Address &ip)
{
	ClusterObjectSerialized::memberOffline(ip);
	std::size_t index = getOnlineClientId(ip);


	if(index < onlineClients.size())
	{
		//Distribute ids across clients
		bool dataToTakeOver = true;
		while(dataToTakeOver)
		{
			dataToTakeOver = false;

			//Lock cluster mutex
			insertMutex.lock();

			onlineClientsMutex.lock();

			//Counting how much data was taken over
			unsigned int counter = 0;

			//Global strings used for new ids being generated
			//And error messages
			string newId;
			string error;
			for(const string id : onlineClients[index].ids)
			{
				list<std::size_t> &ids = idsInClients[id];

				//Deleting id from client who went offline
				auto indexIterator = find(ids.begin(), ids.end(), index);
				if(indexIterator != ids.end())ids.erase(indexIterator);

				//cout<<"Trying to take over "<<id<<endl;

				//Checking if the data is available somewhere else
				if(ids.empty())
				{
					cout<<"Last member of id "<<id<<" went offline"<<endl;
					break;
				}

				//Checking if the data stored on the current client.
				//It doesn't make sense (and is impossible) to store the data twice
				const bool locallyStored = (find(onlineClients[0].ids.cbegin(), onlineClients[0].ids.cend(), id) != onlineClients[0].ids.cend());
				//if(locallyStored)cout<<"Can't take over because I store it"<<endl;
				//if(ids.size() >= dataRedundancy)cout<<"No need to take over. Stored "<<ids.size()<<" of "<<dataRedundancy<<endl;
				if(ids.size() < dataRedundancy && !locallyStored)
				{
					//Fetch data from other clients
					for(std::size_t otherIndex : ids)
					{
						const Address *address = onlineClients[otherIndex].address;
						//cout<<"Asking "<<address->address<<endl;
						if(!address)continue;

						//Ask for package
						Package answer;
						Package pkg;
						pkg<<ClusterObjectDistributedOperation::own;
						pkg<<OwnOperation::fetch;
						pkg<<id;
						ClusterObjectSerialized::askPackage(*address, pkg, &answer);

						//Insert package
						if(performInsert(answer, newId, error))
						{
							cout<<"Took over data from "<<ip.address<<": "<<newId<<endl;

							onlineClients[0].ids.push_back(newId);
							idsInClients[newId].push_back(0);

							//Inform other clients
							Package informPackage;
							informPackage<<ClusterObjectDistributedOperation::own;
							informPackage<<OwnOperation::inserted;
							informPackage<<newId;

							while(!ClusterObjectSerialized::sendPackage(informPackage, nullptr))usleep(1000);

							//Breaking out of loop
							break;
						}
						/*else
						{
							cout<<"Error inserting data: "<<error<<endl;
						}*/
					}

					//Increase the counter of packages being taken over
					if(++counter >= takeOverSize)
					{
						dataToTakeOver = true;
						break;
					}
				}
			}
			onlineClientsMutex.unlock();
			insertMutex.unlock();
		}

		//Set deleted
		onlineClientsMutex.lock();
		onlineClients[index].setDeleted();
		onlineClientsMutex.unlock();
	}
}

bool ClusterObjectDistributed::perform(const Address &address, const Package &p, Package &answer, Package &toSend)
{
	ClusterObjectDistributedOperation type;
	if(!(p>>type))return false;
	switch(type)
	{
	case ClusterObjectDistributedOperation::command:
		return performCommand(p, answer, toSend);
	case ClusterObjectDistributedOperation::own: {
		OwnOperation ownType;
		if(!(p>>ownType))return false;
		switch(ownType)
		{
		case OwnOperation::fetch: {
			std::string id;
			if(!(p>>id))return false;
			performFetch(id, answer);
			return true;
		}
		case OwnOperation::deleted: {
			//ID was inserted in given client
			string id;
			if(!(p>>id))return false;
			std::size_t index = getOnlineClientId(address);
			if(index > onlineClients.size())
			{
				cout<<"Client didn't exist in onlineClients"<<endl;
				return false;
			}

			onlineClientsMutex.lock();
			auto it = find(onlineClients[index].ids.begin(), onlineClients[index].ids.end(), id);
			if(it != onlineClients[index].ids.cend())onlineClients[index].ids.erase(it);

			list<std::size_t> &ids = idsInClients[id];
			auto it2 = find(ids.begin(), ids.end(), index);
			if(it2 != ids.cend())ids.erase(it2);
			onlineClientsMutex.unlock();

			return true;
		}
		case OwnOperation::insert: {
			string id;
			string error;
			list<string> ids;
			while(!p.finished())
			{
				if(performInsert(p, id, error))
				{
					ids.push_back(id);
					onlineClients[0].ids.push_back(id);
					idsInClients[id].push_back(0);
				}
			}

			if(error.empty())
			{
				answer<<true;
				Package message;
				message<<ClusterObjectDistributedOperation::own;
				message<<OwnOperation::inserted;
				for(const string &tempId : ids)message<<tempId;
				while(!ClusterObjectSerialized::sendPackage(message, nullptr))usleep(1000);
			}
			else
			{
				answer<<false;
				answer<<error;
			}
			return true;
		}
		case OwnOperation::inserted: {
			//ID was inserted in given client
			std::size_t index = getOnlineClientId(address);
			onlineClientsMutex.lock();
			if(index > onlineClients.size())
			{
				onlineClients.push_back(ClientRecord(address));
				index = onlineClients.size()-1;
			}
			string id;
			while(p>>id)
			{
//cout<<address.address<<" contains now "<<id<<endl;
				onlineClients[index].ids.push_back(id);
				idsInClients[id].push_back(index);
			}
			onlineClientsMutex.unlock();
			return true;
		}
		case OwnOperation::all_ids:
			for(const string &id : onlineClients[0].ids)
				answer<<id;
			return true;
		default:
			return false;
		}
	}
	default:
		return false;
	}
}

void ClusterObjectDistributed::getRebuildPackage(Package &/*out*/) const
{

}

void ClusterObjectDistributed::rebuild(const Package &/*out*/, const Address &/*address*/)
{

}

void ClusterObjectDistributed::insertData(const Package &data, string &error)
{
	set<std::size_t> tried;
	unsigned int sentCount = 0;

	if(lockOnInsert)insertMutex.lock();

	while(sentCount < dataRedundancy && tried.size() < onlineClients.size())
	{
		//Choose random client to insert
		const std::size_t onlineClientIndex = rand() % getOnlineClientsCount();
		if(!tried.insert(onlineClientIndex).second)continue;	//Don't try to insert multiple times
		const Address *address = getOnlineClientAddress(onlineClientIndex);
		if(address)
		{
			//Create and sendend package to inser data
			Package message;
			message<<ClusterObjectDistributedOperation::own;
			message<<OwnOperation::insert;
			message<<data;
			Package answer;
			ClusterObjectSerialized::askPackage(*address, message, &answer);

			//Check answer
			bool inserted;
			answer>>inserted;
			if(inserted)++sentCount;
			else
			{
				string err;
				answer>>err;
				if(!err.empty())
				{
					//Critical error
					error = err;
					break;
				}
			}
		}
		else
		{
			//Insert data
			string id;
			string err;
			if(!performInsert(data, id, err))
			{
				if(!err.empty())
				{
					//Critical error
					error = err;
					break;
				}
			}
			onlineClientsMutex.lock();
			onlineClients[0].ids.push_back(id);
			idsInClients[id].push_back(0);
			onlineClientsMutex.unlock();

			//Send other client information about insert
			Package message;
			message<<ClusterObjectDistributedOperation::own;
			message<<OwnOperation::inserted;
			message<<id;
			while(!ClusterObjectSerialized::sendPackage(message, nullptr))usleep(1000);

			++sentCount;
		}
	}

	if(lockOnInsert)insertMutex.unlock();
}

void ClusterObjectDistributed::insertData(const list<Package> &data, std::string &error)
{
	//onlineClientsMutex.lock();
	vector<list<Package> > packagesForClient(onlineClients.size());
	vector<bool> success(onlineClients.size());
	//vector<thread*> sendingThreads(onlineClients.size());
	bool criticalError = false;
	mutex errorMutex;

	for(const Package &pkg : data)
	{
		set<std::size_t> tried;
		unsigned int sentCount = 0;

		while(sentCount < dataRedundancy && tried.size() < getOnlineClientsCount())
		{
			const std::size_t onlineClientIndex = rand() % getOnlineClientsCount();
			if(!tried.insert(onlineClientIndex).second)continue;	//Don't try to insert multiple times

			packagesForClient[onlineClientIndex].push_back(pkg);
			++sentCount;
		}
	}

	if(lockOnInsert)insertMutex.lock();

	for(unsigned int i = 0; i < packagesForClient.size(); ++i)
	{
		const unsigned int index = i;
		success[index] = false;
		Address *address = nullptr;
		if(index != 0)address = onlineClients[index].address;

		//TODO for parallel sending either a clustermutex needs to be used,
		//The packages need to be sent unserialized or the information about
		//who contains what needs to be sent centrally
		//sendingThreads[index] = new thread([packagesForClient,&success,index,address,&criticalError,this,&errorMutex,&error] ()
		//{
			if(address)
			{
				//Create and sendend package to insert data
				Package message;
				message<<ClusterObjectDistributedOperation::own;
				message<<OwnOperation::insert;
				for(const Package &pkg : packagesForClient[index])message<<pkg;
				Package answer;
				ClusterObjectSerialized::askPackage(*address, message, &answer);

				//Check answer
				bool s;
				answer>>s;
				if(!(success[index] = s))
				{
					string err;
					answer>>err;
					if(!err.empty())
					{
						//Critical error
						criticalError = true;
						errorMutex.lock();
						error = err;
						errorMutex.unlock();
					}
				}
			}
			else
			{
				//Insert data
				list<string> ids;
				string id;
				string err;
				for(const Package &pkg : packagesForClient[index])
				{
					if(performInsert(pkg, id, err))
					{
						ids.push_back(id);
						onlineClients[0].ids.push_back(id);
						idsInClients[id].push_back(0);
					}
				}

				if(!err.empty())
				{
					//Critical error
					criticalError = true;
					errorMutex.lock();
					error += err;
					errorMutex.unlock();
				}
				else
				{
					//Send other client information about insert
					Package message;
					message<<ClusterObjectDistributedOperation::own;
					message<<OwnOperation::inserted;
					for(const string &currentId : ids)message<<currentId;
					message<<id;
					while(!ClusterObjectSerialized::sendPackage(message, nullptr))usleep(1000);

					success[index] = true;
				}
			}
		//});
	}

	//onlineClientsMutex.unlock();

	//for(unsigned int i = 0; i < packagesForClient.size(); ++i)
	//{
	//	sendingThreads[i]->join();
	//	delete sendingThreads[i];
	//}
	if(lockOnInsert)insertMutex.unlock();

	if(criticalError)return;

	for(unsigned int i = 0; i < success.size(); ++i)
	{
		if(!success[i])
		{
			insertData(packagesForClient[i], error);
		}
	}
}

bool ClusterObjectDistributed::sendPackage(const Package &a, AnswerPackage *answer)
{
	Package message;
	message<<ClusterObjectDistributedOperation::command;
	message<<a;
	return ClusterObjectSerialized::sendPackage(message, answer);
}

bool ClusterObjectDistributed::sendPackageUnserialized(const Package &a, AnswerPackage *answer)
{
	Package message;
	message<<ClusterObjectDistributedOperation::command;
	message<<a;
	return ClusterObjectSerialized::sendPackageUnserialized(message, answer);
}

bool ClusterObjectDistributed::askPackage(const Address &ip, const Package &a, Package *answer)
{
	Package message;
	message<<ClusterObjectDistributedOperation::command;
	message<<a;
	return ClusterObjectSerialized::askPackage(ip, message, answer);
}

bool ClusterObjectDistributed::deleted(const std::string &id)
{
	Package message;
	message<<ClusterObjectDistributedOperation::own;
	message<<OwnOperation::deleted;
	message<<id;
	return ClusterObjectSerialized::sendPackage(message, nullptr);
}
