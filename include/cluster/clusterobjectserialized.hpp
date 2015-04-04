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
	ClusterObjectSerialized(ClusterObject *network, unsigned int maxPackagesToRemember=100);

	/**
	  * Default destructor. Removes the member callback.
	 **/
	virtual ~ClusterObjectSerialized();

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
	virtual void memberOnline(const Address &ip, bool isMaster);

	/**
	  * This function is called whenever a memeber is offline.
	  * It is not used by ClusterObjectSerialized.
	 **/
	virtual void memberOffline(const Address &ip);

	/**
	  * This function sends the given package to the network.
	  * It is possible that this function does not send the
	  * package and returns false because the object is currently
	  * in the phase of rebuilding.
	 **/
	virtual bool sendPackage(const Package &a, Package *answer);

	/**
	  * This function asks the given member of the network.
	 **/
	virtual bool askPackage(const Address &ip, const Package &a, Package *answer);

	/**
	  * This function is called for every Package theat is
	  * received. ClusterObjectSerialized needs to override
	  * this function from CLusterObject to ensure the correct
	  * order of the packages
	 **/
	virtual bool received(const Address &ip, const Package &message, Package &answer, Package &to_send);

private:
	/**
	  * This function is called internally for every Package
	  * which needs to be remembered, either from the network
	  * or the current object
	 **/
	void packageToRemember(const unsigned long long id, const Package &pkg);

	/**
	  * This function is called internally to rebuild the
	  * ClusterObjectSerialized an its subobject
	 **/
	void rebuildAll(const Package &a);

private:
	/**
	  * This list is used to remember the last packages
	  * which are used for other memebers to rebuild
	  * a small part of the object
	 **/
	std::list<std::pair<unsigned long long,Package> > lastPackages;

	/**
	  * The amount of Packages to remember
	 **/
	unsigned int maxPackagesToRemember;

	/**
	  * This mutex is used to synchronize the build
	  * process and sending the packages
	 **/
	std::mutex rebuildMutex;

	/**
	  * This flag indicates whether the object was
	  * rebuild
	 **/
	bool rebuilded;

}; //end class ClusterObjectSerialized

} //end namespace cluster

#endif //CLUSTEROBJECTSERIALIZED_HPP
