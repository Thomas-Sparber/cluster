#ifndef MUTEX_HPP
#define MUTEX_HPP

#include <cluster/clusterobject.hpp>
#include <unistd.h>

namespace cluster
{

class Address;

class ClusterMutex : public ClusterObject
{

private:
	enum ClusterMutexOperation : unsigned char
	{
		lock_mutex = 'l',
		unlock_mutex = 'u'
	};

public:
	ClusterMutex(ClusterObject *network, unsigned int minSleepTime=10) :
		ClusterObject(network),
		tryLock(false),
                selfLocked(false),
                clusterLocked(false),
		minSleepTime(minSleepTime)
	{}

	virtual ~ClusterMutex() {}

	void lock()
	{
		unsigned int counter = 0;
		while(!try_lock())
		{
			if(clusterLocked)
			{
				//counter = 0;
				while(clusterLocked)usleep(minSleepTime);
			}
			else usleep(int(pow(2, counter++) * minSleepTime));
		}
	}

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
		send(lock_mutex, &response);
		if(response.getLength() != 0)	//Mutex can't be locked if one clusternode returns answer
		{
			//TODO only send unlock request to members who locked mutex
			//Save bandwith!
			send(unlock_mutex, &response);

			tryLock = false;
			return false;
		}

		//Mutex is now locked
		selfLocked = true;
		tryLock = false;
		return true;
	}

        void unlock()
        {
                send(unlock_mutex);
                selfLocked = false;
	}

	virtual std::string getType() const
	{
		return "Clustermutex";
	}

private:
	virtual bool received(const Address __attribute__((__unused__)) &ip, const Package &message, Package &answer, Package  __attribute__((__unused__)) &send)
	{
		ClusterMutexOperation type;
		message>>type;

		switch(type)
		{
		case lock_mutex:
			if(tryLock || selfLocked || clusterLocked)
				answer<<'x';
			else
				clusterLocked = true;
			return true;
		case unlock_mutex:
			clusterLocked = false;
			return true;
		default:
			return false;
		}
	}

private:
	bool tryLock;
	bool selfLocked;
	bool clusterLocked;
	unsigned int minSleepTime;

}; // end class ClusterMutex

} // end namespace cluster

#endif
