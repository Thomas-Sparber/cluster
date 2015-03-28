#ifndef MUTEX_HPP
#define MUTEX_HPP

#include <cluster/clusterobject.hpp>
#include <unistd.h>

namespace cluster
{

class Address;

class ClusterMutex : public ClusterObject
{

public:
	ClusterMutex(ClusterObject *network, unsigned int ui_minSleepTime=10) :
		ClusterObject(network),
		tryLock(false),
                selfLocked(false),
                clusterLocked(false),
		minSleepTime(ui_minSleepTime)
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
		send('l', &response);
		if(response.getLength() != 0)	//Mutex can't be locked if one clusternode returns answer
		{
			//TODO only send unlock request to members who locked mutex
			//Save bandwith!
			send('u', &response);

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
                send('u');
                selfLocked = false;
	}

	virtual std::string getType() const
	{
		return "Clustermutex";
	}

private:
	virtual bool received(const Address &/*ip*/, const Package &message, Package &answer, Package &/*to_send*/)
	{
		unsigned char type;
		message>>type;

		switch(type)
		{
		case 'l':	//Lock
			if(tryLock || selfLocked || clusterLocked)
				answer<<'x';
			else
				clusterLocked = true;
			return true;
		case 'u':	//Unlock
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
