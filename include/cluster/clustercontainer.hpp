/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef CLUSTERCONTAINER_HPP
#define CLUSTERCONTAINER_HPP

#include <cluster/clusterobjectserialized.hpp>
#include <cluster/clustercontainerfunctions.hpp>
#include <vector>
#include <list>
#include <mutex>
#include <iostream>

namespace cluster
{

/**
  * This enum defines the actions of the
  * ClusterContainer
 **/
enum class ClusterContainerOperation : char
{
	/**
	  * This action defines to add an element
	  * to the ClusterContainer
	 **/
	add = 'a',

	/**
	  * This action defines to set/replace an element
	  * of the ClusterContainer
	 **/
	set = 's',

	/**
	  * This action defines to remove an element
	  * from the ClusterContainer
	 **/
	erase = 'e'
};

/**
  * This function is overloaded from the Package class
  * to retrieve a ClusterContainerOperation from a Package
 **/
template <>
inline bool operator>>(const Package &p, ClusterContainerOperation &t)
{
	return p>>reinterpret_cast<char&>(t);
}


/**
  * This function is overloaded from the Package class
  * to insert a ClusterContainerOperation into a Package
 **/
template <>
inline void operator<<(Package &p, const ClusterContainerOperation &t)
{
	p<<reinterpret_cast<const char&>(t);
}

/**
  * The class ClusterContainer can be parameterized with
  * any type. IMPORTANT: the operator<<(Package, const T&)
  * needs to be overloaded for classes and structures.
  * A ClusterContainer is a container of objects which
  * is synchronized across a network. This means, if one object
  * is removed on a node, it is also removed on the other nodes.
  * It is advisable to use a ClusterMutex to synchronize
  * access to a ClusterContainer.
  * The ClusterContainer inherits ClusterObjectSerialized in
  * order to make sure that one operation is performed after
  * the other and to make sure that a node which joins the
  * network at a later point of the time, contains the same
  * content as the others. ClusterObjectSerialized is also
  * responsible for handling nodes which lose their connection
  * to the network and join it again later.
 **/
template <class Index, class T, class Container>
class ClusterContainer : public ClusterObjectSerialized
{

public:
	/**
	  * Default constructor which creates the
	  * ClusterContainer and adds it to the given network
	 **/
	ClusterContainer(ClusterObject *network, unsigned int ui_maxPackagesToRemember=100) :
		ClusterObjectSerialized(network, ui_maxPackagesToRemember),
		v(),
		containerMutex()
	{}

	/**
	  * Default destructor
	 **/
	virtual ~ClusterContainer() {}

	/**
	  * Adds an element to the ClusterContainer
	 **/
	bool add(const T &t)
	{
		return doAndSend(ClusterContainerOperation::add, t, 0);
	}

	/**
	  * Sets/replaces an element in the ClusterContainer
	 **/
	bool set(const Index &i, const T &t)
	{
		return doAndSend(ClusterContainerOperation::set, t, i);
	}

	/**
	  * Removes an element from the ClusterContainer
	 **/
	bool erase(const Index &i)
	{
		return doAndSend(ClusterContainerOperation::erase, v[i], i);
	}

	/**
	  * Retrieves an element from the ClusterContainer
	 **/
	const T& get(const Index &i) const
	{
		return getObjectFromContainer<Index, T>(v, i);
	}

	/**
	  * Finds an element in the ClusterContainer
	 **/
	typename Container::iterator find(const T &t)
	{
		return std::find(v.begin(), v.end(), t);
	}

	/**
	  * Finds an element in the ClusterContainer
	 **/
	typename Container::const_iterator find(const T &t) const
	{
		return std::find(v.cbegin(), v.cend(), t);
	}

	/**
	  * Gets the size of the ClusterContainer
	 **/
	std::size_t size() const
	{
		return v.size();
	}

	/**
	  * Checks whether the ClusterContainer is empty
	 **/
	bool empty() const
	{
		return v.empty();
	}

	/**
	  * Returns the type of ClusterObject
	 **/
	virtual std::string getType() const override
	{
		return "ClusterContainer";
	}

protected:
	/**
	  * Overrides the function from ClusterObjectSerialized.
	  * This function is called whenever a package was missed
	  * and needs to be performed.
	 **/
	virtual bool perform(const Address &/*address*/, const Package &message, Package &/*answer*/, Package &/*toSend*/) override
	{
		bool success = true;

		ClusterContainerOperation type;
		T t;
		Index i;

		containerMutex.lock();

		//Extracting all packages
		while(message>>type)
		{
			if(!(message>>t)){ success = false; break; }
			if(!(message>>i)){ success = false; break; }

			success = perform(type, t, i) && success;
		}
		containerMutex.unlock();
		return success;
	}

	/**
	  * Overrides the function from ClusterObjectSerialized.
	  * This function is called whenever a member needs to
	  * rebuild and needs the data. The given package is filled
	  * with the data so that the rebuild function can rebuild the
	  * entire structure using this package.
	 **/
	virtual void getRebuildPackage(Package &out) const override
	{
		//Adding every element to the package
		out<<v;
	}

	/**
	  * Ovedrrides the function from ClusterObjectSerialized.
	  * This function is called whenever the structure needs
	  * to be rebuilt entirely.
	 **/
	virtual void rebuild(const Package &out, const Address &/*address*/) override
	{
		containerMutex.lock();
		out>>v;
		containerMutex.unlock();
	}

private:
	/**
	  * Performs the given operation by first sending it to the
	  * network, waiting for approvement and then performing
	  * it locally.
	 **/
	bool doAndSend(ClusterContainerOperation type, const T &t, const Index &i)
	{
		bool success = false;
		containerMutex.lock();
		if(send(type, t, i, nullptr))
		{
std::cout<<"OK -> ";
			//Only performing operation after approvement of network
			success = perform(type, t, i);
		}
		containerMutex.unlock();
		return success;
	}

	/**
	  * Performs the given operation locally. This function
	  * is called by dAndSend and by received.
	 **/
	bool perform(ClusterContainerOperation type, const T &t, const Index &i)
	{
		switch(type)
		{
		case ClusterContainerOperation::add:
std::cout<<"\t"<<t<<std::endl;
			v.push_back(t);
			return true;
		case ClusterContainerOperation::set:
			replaceObjectInContainer(v, i, t);
			return true;
		case ClusterContainerOperation::erase:
			removeObjectFromContainer(v, i);
			return true;
		default:
			return false;
		}
	}

private:
	/**
	  * This Container stores the data
	 **/
	Container v;

	/**
	  * This mutex is used to synchronize access
	  * to the container
	 **/
	std::mutex containerMutex;

}; // end class ClusterContainer

/**
  * The ClusterList represents a ClusterContainer
  * which uses an std::list for the internal storage
 **/
template<class T, class Index=uint64_t> using ClusterList = ClusterContainer<Index, T, std::list<T> >;

/**
  * The ClusterList represents a ClusterContainer
  * which uses an std::vector for the internal storage
 **/
template<class T, class Index=uint64_t> using ClusterVector = ClusterContainer<Index, T, std::vector<T> >;

} //end namespace cluster

#endif //CLUSTERCONTAINER_HPP
