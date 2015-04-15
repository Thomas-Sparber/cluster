/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#include <cluster/clustermutex.hpp>
#include <unistd.h>
#include <iostream>

using namespace std;
using namespace cluster;

namespace cluster
{

	/**
	  * This enum defines the actions of the
	  * ClusterMutex
	 **/
	enum class ClusterMutexOperation : unsigned char
	{
		/**
		  * Locks the mutex
		 **/
		lock = 'l',

		/**
		  * Unlocks the mutex
		 **/
		unlock = 'u'
	};

	/**
	  * This function is overloaded from the Package class
	  * to retrieve a ClusterMutexOperation from a Package
	 **/
	template <>
	inline bool operator>>(const Package &p, ClusterMutexOperation &t)
	{
		return p>>reinterpret_cast<unsigned char&>(t);
	}


	/**
	  * This function is overloaded from the Package class
	  * to insert a ClusterMutexOperation into a Package
	 **/
	template <>
	inline void operator<<(Package &p, const ClusterMutexOperation &t)
	{
		p<<reinterpret_cast<const unsigned char&>(t);
	}

} //end namespace cluster


ClusterMutex::ClusterMutex(ClusterObject *network, unsigned int ui_minSleepTime, unsigned int ui_resetEvery) :
	ClusterObject(network),
	tryLock(false),
	selfLocked(false),
	clusterLocked(false),
	minSleepTime(ui_minSleepTime),
	resetEvery(ui_resetEvery)
{}

ClusterMutex::~ClusterMutex() {}

void ClusterMutex::lock()
{
	unsigned int counter = 0;
	while(!try_lock())
	{
		if(clusterLocked)
		{
			while(clusterLocked)usleep(minSleepTime);
		}
		else usleep(int(pow(2, counter++) * minSleepTime));

		//reset counter
		if(counter >= resetEvery)counter = 0;
	}
}

bool ClusterMutex::try_lock()
{
	if(selfLocked)return true;	//Mutex already locked

	//This flags prevents concurrent locks
	//by the cluster and the current mutex.
	//If this flag is set to true the mutex
	//doesn't get locked by the cluster.
	tryLock = true;

	if(clusterLocked)	//Unable to lock if mutex is locked by cluster
	{
		tryLock = false;
		return false;
	}

	//Sending the request to the cluster
	AnswerPackage response;
	send(ClusterMutexOperation::lock, &response);

	bool rejected = false;
	for(const auto &package : response)
	{
		unsigned char c;
		while(package.second>>c)
		{
			if(c == 'x')
			{
				rejected = true;
				break;
			}
		}

		if(rejected)break;
	}

	if(rejected)	//Mutex can't be locked if one clusternode returns answer
	{
		//Send unlock request
		send(ClusterMutexOperation::unlock, nullptr);

		tryLock = false;
		return false;
	}

	//Mutex is now locked
	selfLocked = true;
	tryLock = false;
	return true;
}

void ClusterMutex::unlock()
{
	send(ClusterMutexOperation::unlock, nullptr);
	selfLocked = false;
}

bool ClusterMutex::received(const Address &/*ip*/, const Package &message, Package &answer, Package &/*to_send*/)
{
	ClusterMutexOperation type;
	message>>type;

	switch(type)
	{
	case ClusterMutexOperation::lock:	//Lock
		//Locking is only allowed if the mutex is not
		//already locked by the current mutex, the cluster
		//or the current mutex is trying to lock it.
		if(tryLock || selfLocked || clusterLocked)
			answer<<'x';
		else
			clusterLocked = true;
		return true;
	case ClusterMutexOperation::unlock:	//Unlock
		clusterLocked = false;
		return true;
	default:
		return false;
	}
}
