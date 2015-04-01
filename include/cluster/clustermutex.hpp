/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef MUTEX_HPP
#define MUTEX_HPP

#include <cluster/clusterobject.hpp>
#include <unistd.h>

namespace cluster
{

class Address;

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
	return p.getAndNext(reinterpret_cast<unsigned char&>(t));
}


/**
  * This function is overloaded from the Package class
  * to insert a ClusterMutexOperation into a Package
 **/
template <>
inline void operator<<(Package &p, const ClusterMutexOperation &t)
{
	p.append(reinterpret_cast<const unsigned char&>(t));
}

/**
  * The ClusterMutex can be used to synchronize
  * access to an object e.g. ClusterContainer
 **/
class ClusterMutex : public ClusterObject
{

public:
	/**
	  * Constructs a mutex and adds it to the given network.
	  * The minSleepTime determines the minimum sleep time
	  * between retries when the mutex can't be locked.
	  * ResetEvery determines the number of retries when
	  * the sleeptime is reset to the minimum.
	 **/
	ClusterMutex(ClusterObject *network, unsigned int ui_minSleepTime=10, unsigned int ui_resetEvery=17) :
		ClusterObject(network),
		tryLock(false),
                selfLocked(false),
                clusterLocked(false),
		minSleepTime(ui_minSleepTime),
		resetEvery(ui_resetEvery)
	{}

	/**
	  * Default destructor
	 **/
	virtual ~ClusterMutex() {}

	/**
	  * This function locks the mutex. It the mutex
	  * can't be locked the function sleeps for a while
	  * (exponentially more the often it fails to lock)
	  * and retries.
	 **/
	void lock()
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

	/**
	  * This function tries to lock the mutex and
	  * returns whether is was successful or not.
	 **/
	bool try_lock()
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
		Package response;
		send(ClusterMutexOperation::lock, &response);

		bool rejected = false;
		unsigned char c;
		while(response>>c)
		{
			if(c == 'x')
			{
				rejected = true;
				break;
			}
		}

		if(rejected)	//Mutex can't be locked if one clusternode returns answer
		{
			//Send unlock request
			send(ClusterMutexOperation::unlock, &response);

			tryLock = false;
			return false;
		}

		//Mutex is now locked
		selfLocked = true;
		tryLock = false;
		return true;
	}

	/**
	  * This function unlocks the ClusterMutex
	 **/
        void unlock()
        {
                send(ClusterMutexOperation::unlock);
                selfLocked = false;
	}

	/**
	  * Returns whether the mutex is locked
	  * by the cluster or the current mutex
	 **/
	bool isLocked() const
	{
		return selfLocked || clusterLocked;
	}

	/**
	  * Returns whether the mutex is locked
	  * by the current mutex
	 **/
	bool isSelfLocked() const
	{
		return selfLocked;
	}

	/**
	  * Returns whether the mutex is locked
	  * by the cluster
	 **/
	bool isClusterLocked() const
	{
		return clusterLocked;
	}

	/**
	  * Returns the type of ClusterObject
	 **/
	virtual std::string getType() const
	{
		return "Clustermutex";
	}

protected:
	/**
	  * This function is called for every Package
	  * which is received for the ClusterMutex.
	 **/
	virtual bool received(const Address &/*ip*/, const Package &message, Package &answer, Package &/*to_send*/)
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

private:
	/**
	  * A flag that indicates whether the current
	  * mutex is trying to lock it
	 **/
	bool tryLock;

	/**
	  * A flag that indicates whether the mutex
	  * is locked by the current mutex
	 **/
	bool selfLocked;

	/**
	  * A flag that indicates whether the mutex
	  * is locked by the cluster
	 **/
	bool clusterLocked;

	/**
	  * The minimum sleep time between retries
	 **/
	unsigned int minSleepTime;

	/**
	  * The number of retries when the sleeptime
	  * is reset to the minimum
	 **/
	unsigned int resetEvery;

}; // end class ClusterMutex

} // end namespace cluster

#endif
