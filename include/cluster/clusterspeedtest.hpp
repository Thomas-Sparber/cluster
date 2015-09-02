/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef CLUSTERSPEEDTEST_HPP
#define CLUSTERSPEEDTEST_HPP

#include <cluster/clusterobject.hpp>
#include <cluster/prototypes/membercallback.hpp>
#include <mutex>
#include <thread>
#include <map>

namespace cluster
{

/**
  * This class can be used to test the
  * speed of a network
 **/
class ClusterSpeedTest : public ClusterObject, public MemberCallback
{

public:
	/**
	  * The constructor registers
	  * a memberCallback of the ClusterSpeedTest
	  * to be notified when members join the network
	 **/
	ClusterSpeedTest(ClusterObject *network, unsigned int dataSize=100);

	/**
	  * Copy constructor
	 **/
	ClusterSpeedTest(ClusterSpeedTest &t);

	/**
	  * Default destructor. Removes the member callback.
	 **/
	virtual ~ClusterSpeedTest();

	/**
	  * Assignment operator
	 **/
	ClusterSpeedTest& operator= (ClusterSpeedTest &t);

	void start()
	{
		running = true;
		t = new std::thread(&ClusterSpeedTest::sendThread, this);
	}

	void stop()
	{
		running = false;
		t->join();
	}

	bool isRunning()
	{
		return running;
	}

	void printStatistics() const;

protected:
	/**
	  * This function is called whenever a new memeber jois
	  * the network. It checks whether the new member is the master
	  * and rebuilds from it if necessary.
	 **/
	virtual void memberOnline(const Address &ip, bool isMaster) override;

	/**
	  * This function is called whenever a memeber is offline.
	  * It is not used by ClusterSpeedTest.
	 **/
	virtual void memberOffline(const Address &ip) override;

	/**
	  * This function is called for every Package theat is
	  * received. ClusterSpeedTest needs to override
	  * this function from CLusterObject to ensure the correct
	  * order of the packages
	 **/
	virtual bool received(const Address &ip, const Package &message, Package &answer, Package &to_send) override;

	/**
	  * Returns the type of ClusterObject
	 **/
	virtual std::string getType() const
	{
		return "Speedtest";
	}

private:
	/**
	  * Generates a string with random data
	 **/
	void fillRandomPackage(Package &p, unsigned int length) const;

	/**
	  * This function is called by the worker thread to send packages
	 **/
	void sendThread();

private:
	/**
	  * The amount of data to be sent at once
	 **/
	unsigned int dataSize;

	/**
	  * Package sending speed for each member
	 **/
	std::list<std::pair<Address*, std::list<double> > > packagesSent;

	/**
	  * Synchronizes access to the map
	 **/
	mutable std::mutex mapMutex;

	/**
	  * A flag that indicates the running state
	 **/
	bool running;

	/**
	  * Sends packages all the time
	 **/
	std::thread *t;

}; //end class ClusterSpeedTest

} //end namespace cluster

#endif //CLUSTERSPEEDTEST_HPP
