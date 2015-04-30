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
	enum class ClusterObjectSerializedOperation : char
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
		return p>>reinterpret_cast<char&>(t);
	}


	/**
	  * This function is overloaded from the Package class
	  * to insert a ClusterObjectSerializedOperation into a Package
	 **/
	template <>
	inline void operator<<(Package &p, const ClusterObjectSerializedOperation &t)
	{
		p<<reinterpret_cast<const char&>(t);
	}

	/**
	  * This enum defines the type of message
	 **/
	enum class ClusterObjectSerializedType : char
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
		return p>>reinterpret_cast<char&>(t);
	}


	/**
	  * This function is overloaded from the Package class
	  * to insert a ClusterObjectSerializedType into a Package
	 **/
	template <>
	inline void operator<<(Package &p, const ClusterObjectSerializedType &t)
	{
		p<<reinterpret_cast<const char&>(t);
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
		rebuildAll(answer, ip);
		rebuilded = true;
	}
	rebuildMutex.unlock();
}

void ClusterObjectSerialized::memberOffline(const Address &/*ip*/)
{}

bool ClusterObjectSerialized::sendPackage(const Package &a, AnswerPackage *answer)
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

bool ClusterObjectSerialized::sendPackageUnserialized(const Package &a, AnswerPackage *answer)
{
	Package message;
	message<<ClusterObjectSerializedType::other_ask;
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

bool ClusterObjectSerialized::received(const Address &ip, const Package &message, Package &answer, Package &to_send)
{
	bool success = false;

	ClusterObjectSerializedType type;
	ClusterObjectSerializedOperation operation;
	if(!(message>>type))return false;
	switch(type)
	{
	case ClusterObjectSerializedType::mine:
		if(!(message>>operation))return false;
		switch(operation)
		{
		case ClusterObjectSerializedOperation::get_package:
		{
			unsigned long long id;
			if(!(message>>id))return false;
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
			std::size_t length = 0;
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
		success = true;
		break;
	case ClusterObjectSerializedType::other_ask:
		//Ask packages don't need to be remembered
		success = perform(ip, message, answer, to_send);
		break;
	case ClusterObjectSerializedType::other:
	{
		unsigned long long id;
		if(!(message>>id))return false;

		rebuildMutex.lock();

		unsigned long long checkId = 0;
		if(!lastPackages.empty())checkId = lastPackages.back().first + 1;

		//Check if no errors happened
		if(id > checkId)
		{
//			std::cout<<"Missed package! Asking: "<<checkId<<" to "<<(id-1)<<std::endl;

			bool needToRebuild = true;
			rebuilded = true;

			Package tempAnswer;
			Package tempToSend;

			//Oops we missed at least one package!
			//Get packages before we perform current package
			for(unsigned long long i = checkId; i < id; ++i)
			{
				tempAnswer.clear();
				tempToSend.clear();

				needToRebuild = true;

				AnswerPackage a;
				Package toSend;
				toSend<<ClusterObjectSerializedType::mine;
				toSend<<ClusterObjectSerializedOperation::get_package;
				toSend<<i;
				ClusterObject::sendPackage(toSend, &a);

				bool first = true;
				auto firstPackage = a.cbegin();
				for(auto it = a.cbegin(); it != a.cend(); ++it)
				{
					const Package &pkg = it->second;
					if(first)
					{
						if(pkg.emptyOrNull())break;
						else needToRebuild = false;

						packageToRemember(i, pkg);
						perform(*it->first, pkg, tempAnswer, tempToSend);

						first = false;
					}
					else
					{
						if(pkg != firstPackage->second)
						{
							//Whole (local) network is corrupted. Rebuilding...
							needToRebuild = true;
							break;
						}
					}
				}

				//Package is missing or packages are not equal
				if(needToRebuild)break;
			}

			if(needToRebuild)
			{
//				std::cout<<"Rebuilding from "<<ip.address<<std::endl;
				Package p;
				Package a;
				p<<ClusterObjectSerializedType::mine;
				p<<ClusterObjectSerializedOperation::full_data;
				ClusterObject::askPackage(ip, p, &a);
				rebuildAll(a, ip);
				success = true;
			}
			else
			{
				//Remember package if we managed to rebuild the object
				packageToRemember(id, message.subPackageFromCurrentPosition());
				success = perform(ip, message, answer, to_send);
			}
		}
		else if(id < checkId)
		{
//			std::cout<<"Someone sent a wrong package. Ignoring: "<<id<<". Expected "<<checkId<<std::endl;
			//Returning true which means that the
			//Package was for ClusterObjectSerialized.
			//This means that the child object doesn't
			//perform this package
			success = true;
		}
		else
		{
			//Only remembering correct packages
//std::cout<<id;
			packageToRemember(id, message.subPackageFromCurrentPosition());
			success = perform(ip, message, answer, to_send);
			rebuilded = false;
		}

		rebuildMutex.unlock();

		break;
	}
	default:
		//Error
		success = false;
		break;
	}

	return success;
}

void ClusterObjectSerialized::packageToRemember(const unsigned long long id, const Package &pkg)
{
	lastPackages.push_back(std::pair<unsigned long long,Package>(id, pkg));
	while(lastPackages.size() > maxPackagesToRemember)lastPackages.pop_front();
}

void ClusterObjectSerialized::rebuildAll(const Package &a, const Address &address)
{
	lastPackages.clear();

	unsigned int length;
	unsigned long long id;
	char *data;

	//Read all Packages to remember
	if(!(a>>length))return;
	if(length > 0)
	{
		data = new char[length];
		if(!(a>>id))return;
		a.getAndNext(data, length);
		Package p;
		p.write(data, length);
		lastPackages.push_back(std::pair<unsigned long long,Package>(id, p));
		delete [] data;
	}

	//Rebuild subobject
	rebuild(a, address);

//std::cout<<"Last rebuild id was "<<id<<std::endl;
std::cout<<"Rebuilded"<<std::endl;
}
