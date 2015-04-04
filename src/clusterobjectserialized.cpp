/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#include <cluster/clusterobjectserialized.hpp>
#include <iostream>

using namespace std;
using namespace cluster;

namespace cluster
{
	/**
	  * This enum defines the actions of the
	  * ClusterObjectSerialized
	 **/
	enum class ClusterObjectSerializedOperation : unsigned char
	{
		/**
		  * Full data is used to retireve or send
		  * a Package that can be used to rebuild
		 **/
		full_data = 'f',

		/**
		  * Get package is used when a member missed
		  * one or few packages and needs to rebuild it
		 **/
		get_package = 'g'
	};

	/**
	  * This function is overloaded from the Package class
	  * to retrieve a ClusterObjectSerializedOperation from a Package
	 **/
	template <>
	inline bool operator>>(const Package &p, ClusterObjectSerializedOperation &t)
	{
		return p.getAndNext(reinterpret_cast<unsigned char&>(t));
	}


	/**
	  * This function is overloaded from the Package class
	  * to insert a ClusterObjectSerializedOperation into a Package
	 **/
	template <>
	inline void operator<<(Package &p, const ClusterObjectSerializedOperation &t)
	{
		p.append(reinterpret_cast<const unsigned char&>(t));
	}

	/**
	  * This enum defines the type of message
	 **/
	enum class ClusterObjectSerializedType : unsigned char
	{
		/**
		  * Indicates that the message is intended
		  * for the child object
		 **/
		other = 'o',

		/**
		  * Indicates that the message is intended
		  * for the current object
		 **/
		mine = 'm',

		/**
		  * Indicates that the message is an ask
		  * package for the child object
		 **/
		other_ask = 'a'
	};

	/**
	  * This function is overloaded from the Package class
	  * to retrieve a ClusterObjectSerializedType from a Package
	 **/
	template <>
	inline bool operator>>(const Package &p, ClusterObjectSerializedType &t)
	{
		return p.getAndNext(reinterpret_cast<unsigned char&>(t));
	}


	/**
	  * This function is overloaded from the Package class
	  * to insert a ClusterObjectSerializedType into a Package
	 **/
	template <>
	inline void operator<<(Package &p, const ClusterObjectSerializedType &t)
	{
		p.append(reinterpret_cast<const unsigned char&>(t));
	}

} //end namespace cluster

ClusterObjectSerialized::ClusterObjectSerialized(ClusterObject *network, unsigned int ui_maxPackagesToRemember) :
	ClusterObject(network),
	lastPackages(),
	maxPackagesToRemember(ui_maxPackagesToRemember),
	rebuildMutex(),
	rebuilded(false)
{
	addMemberCallback(this);
}

ClusterObjectSerialized::~ClusterObjectSerialized()
{
	removeMemberCallback(this);
}

void ClusterObjectSerialized::memberOnline(const Address &ip, bool isMaster)
{
	//Only read last actions if new member is master
	if(!isMaster)return;

	//Ask master for rebuild data
	rebuildMutex.lock();
	if(!rebuilded)
	{
//		std::cout<<"Rebuilding from master "<<ip.address<<std::endl;
		Package p;
		Package answer;
		p<<ClusterObjectSerializedType::mine;
		p<<ClusterObjectSerializedOperation::full_data;
		ClusterObject::askPackage(ip, p, &answer);
		rebuildAll(answer);
		rebuilded = true;
	}
	rebuildMutex.unlock();
}

void ClusterObjectSerialized::memberOffline(const Address &/*ip*/)
{}

bool ClusterObjectSerialized::sendPackage(const Package &a, Package *answer)
{
//std::cout<<"Sending "<<a.toString()<<std::endl;
	rebuildMutex.lock();
	const bool localRebuilded = rebuilded;
	rebuilded = false;

	unsigned long long id = 0;
	if(!lastPackages.empty())id = lastPackages.back().first + 1;

	//Check if in the phase of rebuilding
	if(localRebuilded)
	{
		rebuildMutex.unlock();
//std::cout<<"Forgetting package ("<<id<<")"<<std::endl;
		return false;
	}

	//Not rebuilding, so package is legal and can be remembered
	packageToRemember(id, a);
	rebuildMutex.unlock();

//std::cout<<"Sent "<<a.toString()<<" ("<<id<<")"<<std::endl;
	Package message;
	message<<ClusterObjectSerializedType::other;
	message<<id;
	message<<a;
	return ClusterObject::ClusterObject_send(addCurrentSignature(message), answer);
}

bool ClusterObjectSerialized::askPackage(const Address &ip, const Package &a, Package *answer)
{
	Package message;
	message<<ClusterObjectSerializedType::other_ask;
	message<<a;
	return ClusterObject::ClusterObject_ask(ip, addCurrentSignature(message), answer);
}

bool ClusterObjectSerialized::received(const Address &ip, const Package &message, Package &answer, Package &/*to_send*/)
{
	bool messageConsumed = false;

	ClusterObjectSerializedType type;
	ClusterObjectSerializedOperation operation;
	message>>type;
	switch(type)
	{
	case ClusterObjectSerializedType::mine:
		message>>operation;
		switch(operation)
		{
		case ClusterObjectSerializedOperation::get_package:
		{
			unsigned long long id;
			message>>id;
			Package *p = nullptr;
			rebuildMutex.lock();
			for(auto &pkg : lastPackages)
			{
				if(id == pkg.first)
				{
					p = &pkg.second;
					break;
				}
			}

			//Desired package found
			if(p)answer<<(*p);

			rebuildMutex.unlock();
			break;
		}
		case ClusterObjectSerializedOperation::full_data:
		{
			rebuildMutex.lock();
			unsigned int length = 0;
			if(lastPackages.empty())answer<<length;
			else
			{
				length = lastPackages.back().second.getLength();
				answer<<length;
				answer<<lastPackages.back().first;
				answer<<lastPackages.back().second;
			}
			getRebuildPackage(answer);
			rebuildMutex.unlock();
			break;
		}
		default:
			//Error
			break;
		}
		messageConsumed = true;
		break;
	case ClusterObjectSerializedType::other_ask:
		//Ask packages don't need to be remembered
		messageConsumed = false;
		break;
	case ClusterObjectSerializedType::other:
	{
		unsigned long long id;
		message>>id;

		rebuildMutex.lock();

		unsigned long long checkId = 0;
		if(!lastPackages.empty())checkId = lastPackages.back().first + 1;

		//Check if no errors happened
		if(id > checkId)
		{
//			std::cout<<"Missed package! Asking: "<<checkId<<" to "<<(id-1)<<std::endl;

			bool needToRebuild = true;
			rebuilded = true;

			//Oops we missed at least one package!
			//Get packages before we perform current package
			for(unsigned long long i = checkId; i < id; ++i)
			{
				Package a;
				Package toSend;
				toSend<<ClusterObjectSerializedType::mine;
				toSend<<ClusterObjectSerializedOperation::get_package;
				toSend<<i;
				ClusterObject::sendPackage(toSend, &a);

				if(a.emptyOrNull())break;
				else needToRebuild = false;

				packageToRemember(i, a);
				if(!perform(a))
				{
					//the whole (local) network is corrupted stopping and rebuilding
					break;
				}
			}

			if(needToRebuild)
			{
//				std::cout<<"Rebuilding from "<<ip.address<<std::endl;
				Package p;
				Package a;
				p<<ClusterObjectSerializedType::mine;
				p<<ClusterObjectSerializedOperation::full_data;
				ClusterObject::askPackage(ip, p, &a);
				rebuildAll(a);
			}
			else
			{
				//Remember package if we managed to rebuild the object
				packageToRemember(id, message.subPackageFromCurrentPosition());
			}
			messageConsumed = false;
		}
		else if(id < checkId)
		{
//			std::cout<<"Someone sent a wrong package. Ignoring: "<<id<<". Expected "<<checkId<<std::endl;
			//Returning true which means that the
			//Package was for ClusterObjectSerialized.
			//This means that the child object doesn't
			//perform this package
			messageConsumed = true;
		}
		else
		{
			//Only remembering correct packages
//std::cout<<id;
			packageToRemember(id, message.subPackageFromCurrentPosition());
			messageConsumed = false;
			rebuilded = false;
		}

		rebuildMutex.unlock();

		break;
	}
	default:
		//Error
		messageConsumed = false;
		break;
	}

	return messageConsumed;
}

void ClusterObjectSerialized::packageToRemember(const unsigned long long id, const Package &pkg)
{
	lastPackages.push_back(std::pair<unsigned long long,Package>(id, pkg));
	while(lastPackages.size() > maxPackagesToRemember)lastPackages.pop_front();
}

void ClusterObjectSerialized::rebuildAll(const Package &a)
{
	lastPackages.clear();

	unsigned int length;
	unsigned long long id;
	char *data;

	//Read all Packages to remember
	a>>length;
	if(length > 0)
	{
		data = new char[length];
		a>>id;
		a.getAndNext(data, length);
		Package p;
		p.append(data, length);
		lastPackages.push_back(std::pair<unsigned long long,Package>(id, p));
		delete [] data;
	}

	//Rebuild subobject
	rebuild(a);

//std::cout<<"Last rebuild id was "<<id<<std::endl;
std::cout<<"Rebuilded"<<std::endl;
}
