/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef CLUSTEROBJECTSERIALIZED_HPP
#define CLUSTEROBJECTSERIALIZED_HPP

#include <cluster/clusterobject.hpp>
#include <cluster/prototypes/membercallback.hpp>
#include <list>
#include <mutex>

namespace cluster
{

/**
  * This class is responsible for a correct sequence
  * of the packages. This class also handles rebuilds,
  * either partial or complete, if a new member has joined
  * the network or some packages were lost.
  * It is useful to inherit from this class whenever an object
  * needs the packages in correct order and complete.
 **/
class ClusterObjectSerialized : public ClusterObject, public MemberCallback
{

public:
	/**
	  * The constructor can be called giving the amount
	  * of packages to remeber. The constructor registers
	  * a memberCallback of the ClusterObjectSerialized
	  * to be notified when members join the network
	 **/
	ClusterObjectSerialized(ClusterObject *network, unsigned int ui_maxPackagesToRemember=100) :
		ClusterObject(network),
		lastPackages(),
		maxPackagesToRemember(ui_maxPackagesToRemember),
		rebuildMutex(),
		rebuilded(false)
	{
		addMemberCallback(this);
	}

	/**
	  * Default destructor. Removes the member callback.
	 **/
	~ClusterObjectSerialized()
	{
		removeMemberCallback(this);
	}

protected:
	/**
	  * A class which inherits from this class needs
	  * to override this function. This function is called
	  * whenever a package was missed and needs to be
	  * performed.
	  * The parameter p is a package wich consists of the
	  * concatenation of the packages from all members
	  * of the network. So it is a good idea to check if
	  * the content of all packages identical. If not,
	  * the network could be corrupt.
	 **/
	virtual bool perform(const Package &p) = 0;

	/**
	  * A class which inherits from this class needs
	  * to override this function. This function is called
	  * whenever a member needs to rebuild and needs
	  * the data. The given package needs to be filled
	  * so that the rebuild function can rebuild the
	  * entire structure using this package.
	 **/
	virtual void getRebuildPackage(Package &out) = 0;

	/**
	  * A class which inherits from this class needs
	  * to override this function. This function is called
	  * whenever the structure needs to be rebuilt entirely.
	 **/
	virtual void rebuild(const Package &out) = 0;

	/**
	  * This function is called whenever a new memeber jois
	  * the network. It checks whether the new member is the master
	  * and rebuilds from it if necessary.
	 **/
	virtual void memberOnline(const Address &ip, bool isMaster)
	{
		//Only read last actions if new member is master
		//This means that it contains more members of the P2P
		//network as we do.
		if(!isMaster)return;

		//Ask master for rebuild data
		rebuildMutex.lock();
		std::cout<<"Rebuilding from master "<<ip.address<<std::endl;
		Package p;
		Package a;
		p<<'m';	//message_mine
		p<<'f';	//message_full_data
		ClusterObject::askPackage(ip, p, &a);
		rebuildAll(a);
		rebuilded = true;
		rebuildMutex.unlock();
	}

	/**
	  * This function is called whenever a memeber is offline.
	  * It is not used by ClusterObjectSerialized.
	 **/
	virtual void memberOffline(const Address &/*ip*/) {}

	/**
	  * This function sends the given package to the network.
	  * It is possible that this function does not send the
	  * package and returns false because the object is currently
	  * in the phase of rebuilding.
	 **/
	virtual bool sendPackage(const Package &a, Package *answer)
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
		message<<'o';	//message_other
		message<<id;
		message<<a;
		return ClusterObject::ClusterObject_send(addCurrentSignature(message), answer);
	}

	virtual bool askPackage(const Address &ip, const Package &a, Package *answer)
	{
		Package message;
		message<<'a';	//message_other_ask
		message<<a;
		return ClusterObject::ClusterObject_ask(ip, addCurrentSignature(message), answer);
	}

	virtual bool received(const Address &ip, const Package &message, Package &answer, Package &/*to_send*/)
	{
		bool messageConsumed = false;

		unsigned char type;
		message>>type;
		switch(type)
		{
		case 'm':	//message_mine
			message>>type;
			switch(type)
			{
			case 'g':	//message_get_package
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
			case 'f':	//message_full_data
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
				rebuildMutex.unlock();
				getRebuildPackage(answer);
				break;
			}
			default:
				//Error
				break;
			}
			messageConsumed = true;
			break;
		case 'a':	//message_other_ask
			//Ask packages don't need to be remembered
			messageConsumed = true;
			break;
		case 'o':	//message_other
		{
			unsigned long long id;
			message>>id;

			rebuildMutex.lock();

			unsigned long long checkId = 0;
			if(!lastPackages.empty())checkId = lastPackages.back().first + 1;

			//Check if no errors happened
			if(id > checkId)
			{
				std::cout<<"Missed package! Asking: "<<checkId<<" to "<<id<<std::endl;

				bool needToRebuild = true;
				rebuilded = true;

				//Oops we missed at least one package!
				//Get packages before we perform current package
				for(unsigned long long i = checkId; i < id; ++i)
				{
					Package a;
					Package toSend;
					toSend<<'m';	//message_mine
					toSend<<'g';	//message_get_package
					toSend<<i;
					ClusterObject::sendPackage(toSend, &a);

					if(a.empty())break;
					else needToRebuild = false;

					packageToRemember(i, a);
					if(!perform(a))
					{
						std::cout<<"Network corrupted!"<<std::endl;
						//Oops we are in big trouble,
						//the whole network is corrupted
					}
				}

				if(needToRebuild)
				{
					std::cout<<"Rebuilding from "<<ip.address<<std::endl;
					Package p;
					Package a;
					p<<'m';	//message_mine
					p<<'f';	//message_full_data
					ClusterObject::askPackage(ip, p, &a);
					rebuildAll(a);
				}
				messageConsumed = false;
			}
			else if(id < checkId)
			{
				std::cout<<"Someone sent a wrong package. Ignoring: "<<id<<". Expected "<<checkId<<std::endl;
				//Returning true which means that the
				//Package was for ClusterObjectSerialized.
				//This means that the child object doesn't
				//perform this package
				messageConsumed = true;
			}
			else
			{
				//Only remembering correct packages
std::cout<<id;
				packageToRemember(id, message.subPackageFromCurrentPosition());
				messageConsumed = false;
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

private:
	void packageToRemember(const unsigned long long id, const Package &pkg)
	{
		lastPackages.push_back(std::pair<unsigned long long,Package>(id, pkg));
		while(lastPackages.size() > maxPackagesToRemember)lastPackages.pop_front();
	}

	void rebuildAll(const Package &a)
	{
		lastPackages.clear();

		unsigned int length;
		unsigned long long id;
		char *data;

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

std::cout<<"Last rebuild id was "<<id<<std::endl;

		rebuild(a);
	}

private:
	std::list<std::pair<unsigned long long,Package> > lastPackages;
	unsigned int maxPackagesToRemember;
	std::mutex rebuildMutex;
	bool rebuilded;

}; //end class ClusterObjectSerialized

} //end namespace cluster

#endif //CLUSTEROBJECTSERIALIZED_HPP
