/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef CLUSTEROBJECTDISTRIBUTED_HPP
#define CLUSTEROBJECTDISTRIBUTED_HPP

#include <cluster/clusterobjectserialized.hpp>
#include <cluster/clustermutex.hpp>
#include <list>
#include <map>
#include <mutex>

namespace cluster
{

/**
  * This struct is used to keep track which client
  * holds which data
 **/
struct ClientRecord
{

	/**
	  * Constructs a client record for the local client
	 **/
	ClientRecord(bool /*__fake__*/) :
		address(nullptr),
		ids()
	{}

	/**
	  * Constructs a client record for the given address
	 **/
	ClientRecord(const Address &a_address) :
		address(a_address.clone()),
		ids()
	{}

	/**
	  * Copy constructor
	 **/
	ClientRecord(const ClientRecord &r) :
		address(r.address ? r.address->clone() : nullptr),
		ids(r.ids)
	{}

	/**
	  * Destructor
	 **/
	~ClientRecord()
	{
		delete address;
	}

	/**
	  * Assignment operator
	 **/
	ClientRecord& operator= (const ClientRecord &r)
	{
		delete address;
		address = r.address ? r.address->clone() : nullptr,
		ids = r.ids;
		return (*this);
	}

	bool operator== (const Address &a) const
	{
		if(!address)return false;
		return ((*address) == a);
	}

	bool operator== (const ClientRecord &r) const
	{
		if(!address && !r.address)return true;
		if(!address)return false;
		if(!r.address)return false;
		return ((*address) == (*r.address));
	}

	bool isForLocalClient() const
	{
		return (address == nullptr);
	}

	void setDeleted()
	{
		delete address;
		address = nullptr;
		ids.clear();
	}

	/**
	  * The address the client record is holding th data for
	 **/
	Address *address;

	/**
	  * The ids the client holds
	 **/
	std::list<std::string> ids;

}; //end struct clientRecord

/**
  * This class is responsible for sharing data
  * across cluster nodes.
 **/
class ClusterObjectDistributed : public ClusterObjectSerialized
{

public:
	/**
	  * The constructor can be called giving the amount
	  * of packages to remeber for ClusterObjectSerialized.
	  * The constructor registers a memberCallback of the
	  * ClusterObjectDistributed to be notified when members
	  * join and leave the network to be able to redistribute
	  * the data
	 **/
	ClusterObjectDistributed(ClusterObject *network, unsigned int dataRedundancy, unsigned int takeOverSize, unsigned int maxPackagesToRemember=100);

	/**
	  * Default destructor. Removes the member callback.
	 **/
	virtual ~ClusterObjectDistributed();

	/**
	  * Gets the id of the online client with the given address
	  * 0 is reserved for the local client
	 **/
	std::size_t getOnlineClientId(const Address &ip) const;

	/**
	  *
	 **/
	const Address* getOnlineClientAddress(std::size_t index) const
	{
		onlineClientsMutex.lock();
		const Address *a = index < onlineClients.size() ? onlineClients[index].address : nullptr;
		onlineClientsMutex.unlock();
		return a;
	}

	/**
	  * Gets the id of the local client (0)
	 **/
	std::size_t getLocalClientId() const
	{
		return 0;
	}

	/**
	  * Tells the ClusterObjectDistributed that the object
	  * holds the given ids
	 **/
	void addIdsToLocalClient(const std::list<std::string> &ids);

	/**
	  * Tells the ClusterObjectDistributed that the object
	  * holds the given ids from the beginning
	 **/
	void setInitialIds(const std::list<std::string> &ids);

	/**
	  * Returns the amount of clients currently online
	 **/
	std::size_t getOnlineClientsCount() const
	{
		return onlineClients.size();
	}

	/**
	  * This function sends a delete package to the network
	 **/
	bool deleted(const std::string &id);

	/**
	  * Inserts the given id with the given data into
	  * random clients
	 **/
	void insertData(const Package &data, std::string &error);

protected:
	/**
	  * This function needs to be overridden. It is called
	  * whenever a command package needs to be performed
	 **/
	virtual bool performCommand(const Package &p, Package &answer, Package &toSend) = 0;

	/**
	  * This function needs to be overridden. It is called
	  * whenever data needs to be inserted. The function should
	  * return whether the insertion was successful. If error
	  * is set with an error message, the error is assumed to be
	  * critical globally (e.g. primary key constraint violation)
	 **/
	virtual bool performInsert(const Package &data, std::string &idOut, std::string &error) = 0;

	/**
	  * This function is called whenever some data need to
	  * be fetched
	 **/
	virtual bool performFetch(const std::string &id, Package &answer) = 0;

	/**
	  * This function is called whenever a package
	  * needs to be performed.
	 **/
	virtual bool perform(const Address &address, const Package &p, Package &answer, Package &toSend) override;

	/**
	  * A class which inherits from this class needs
	  * to override this function. This function is called
	  * whenever a member needs to rebuild and needs
	  * the data. The given package needs to be filled
	  * so that the rebuild function can rebuild the
	  * entire structure using this package.
	 **/
	virtual void getRebuildPackage(Package &out) const override;

	/**
	  * A class which inherits from this class needs
	  * to override this function. This function is called
	  * whenever the structure needs to be rebuilt entirely.
	 **/
	virtual void rebuild(const Package &out, const Address &address) override;

	/**
	  * This function is called whenever a new memeber jois
	  * the network. It checks whether the new member is the master
	  * and rebuilds from it if necessary.
	 **/
	virtual void memberOnline(const Address &ip, bool isMaster) override;

	/**
	  * This function is called whenever a memeber is offline.
	  * It is not used by ClusterObjectDistributed.
	 **/
	virtual void memberOffline(const Address &ip) override;

	/**
	  * This function sends the given package to the network.
	  * It is possible that this function does not send the
	  * package and returns false because the object is currently
	  * in the phase of rebuilding.
	 **/
	virtual bool sendPackage(const Package &a, AnswerPackage *answer) override;

	/**
	  * This function sends the given package to the network.
	 **/
	virtual bool sendPackageUnserialized(const Package &a, AnswerPackage *answer) override;

	/**
	  * This function asks the given member of the network.
	 **/
	virtual bool askPackage(const Address &ip, const Package &a, Package *answer) override;

private:
	/**
	  * The amount of nodes where data should
	  * be stored redundant. 1 means no redundancy
	 **/
	unsigned int dataRedundancy;

	/**
	  * Defaines the amount of data which is taken over
	  * at once when a client goes offline
	 **/
	unsigned int takeOverSize;

	/**
	  * This vector records the clients which are currently online
	 **/
	std::vector<ClientRecord> onlineClients;

	/**
	  * This map represents the ids which are hold by which client
	 **/
	std::map<std::string,std::list<std::size_t> > idsInClients;

	/**
	  * This mutex prevents concurrent access to onlineClients
	 **/
	mutable std::mutex onlineClientsMutex;

	/**
	  * This clustermutex is used for synchronization
	  * for data takeover when a client goes offline
	 **/
	ClusterMutex takeOverMutex;

}; //end class ClusterObjectDistributed

} //end namespace cluster

#endif //CLUSTEROBJECTDISTRIBUTED_HPP
