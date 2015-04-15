/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef CLUSTERMUTEX_HPP
#define CLUSTERMUTEX_HPP

#include <cluster/clusterobject.hpp>

namespace cluster
{

class Address;

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
	ClusterMutex(ClusterObject *network, unsigned int minSleepTime=10, unsigned int resetEvery=17);

	/**
	  * Default destructor
	 **/
	virtual ~ClusterMutex();

	/**
	  * This function locks the mutex. It the mutex
	  * can't be locked the function sleeps for a while
	  * (exponentially more the often it fails to lock)
	  * and retries.
	 **/
	void lock();

	/**
	  * This function tries to lock the mutex and
	  * returns whether is was successful or not.
	 **/
	bool try_lock();

	/**
	  * This function unlocks the ClusterMutex
	 **/
	void unlock();

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
	virtual std::string getType() const override
	{
		return "Clustermutex";
	}

protected:
	/**
	  * This function is called for every Package
	  * which is received for the ClusterMutex.
	 **/
	virtual bool received(const Address &ip, const Package &message, Package &answer, Package &to_send) override;

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

#endif //CLUSTERMUTEX_HPP
