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

namespace cluster
{

/**
  * This enum defines the actions of the
  * ClusterContainer
 **/
enum class ClusterContainerOperation : unsigned char
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
	return p.getAndNext(reinterpret_cast<unsigned char&>(t));
}


/**
  * This function is overloaded from the Package class
  * to insert a ClusterContainerOperation into a Package
 **/
template <>
inline void operator<<(Package &p, const ClusterContainerOperation &t)
{
	p.append(reinterpret_cast<const unsigned char&>(t));
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
		v()
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
		return getObjectFromContainer<T>(v, i);
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
	unsigned int size() const
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
	virtual std::string getType() const
	{
		return "Clustercontainer";
	}

protected:
	/**
	  * This fucntion is called whenever a package
	  * is received from the network. This function
	  * calls the received method of the ClusterObjectSerialized
	  * to ensure the correct order.
	 **/
	virtual bool received(const Address &ip, const Package &message, Package &answer, Package &to_send)
	{
		//Call receved function of ClusterObjectSerialized.
		//Return true if message is consumed by the ClusterObjectSerialized
		if(ClusterObjectSerialized::received(ip, message, answer, to_send))return true;

		bool success = true;
		ClusterContainerOperation type;
		T t;
		Index i;

		//Extract messages
		while(message>>type)
		{
			if(!(message>>t)){ success = false; break; }
			if(!(message>>i)){ success = false; break; }

			//Perform messages
			success = perform(type, t, i) && success;
		}
		return success;
	}

	/**
	  * Overrides the function from ClusterObjectSerialized.
	  * This function is called whenever a package was missed
	  * and needs to be performed.
	 **/
	virtual bool perform(const Package &message)
	{
		bool success = true;

		//Used to remember the previous objects of the package
		//and to check if the packages are identical from
		//all members of the network.
		ClusterContainerOperation previousType;
		T previousT;
		unsigned int previousI;

		ClusterContainerOperation type;
		T t;
		Index i;

		//Extracting all packages
		bool first = true;
		while(message>>type)
		{
			if(!(message>>t)){ success = false; break; }
			if(!(message>>i)){ success = false; break; }

			if(first)
			{
				first = false;
				success = perform(type, t, i) && success;
			}
			else
			{
				//Check if all packages are identical
				if(previousType != type)success = false;
				if(previousI != i)success = false;
				if(previousT != t)success = false;
			}

			previousType = type;
			previousI = i;
			previousT = t;
		}

		return success;
	}

	/**
	  * Overrides the function from ClusterObjectSerialized.
	  * This function is called whenever a member needs to
	  * rebuild and needs the data. The given package is filled
	  * with the data so that the rebuild function can rebuild the
	  * entire structure using this package.
	 **/
	virtual void getRebuildPackage(Package &out)
	{
		//Adding every element to the package
		for(const T &t : v)
		{
			out<<t;
		}
	}

	/**
	  * Ovedrrides the function from ClusterObjectSerialized.
	  * This function is called whenever the structure needs
	  * to be rebuilt entirely.
	 **/
	virtual void rebuild(const Package &out)
	{
		v.clear();
		T t;
		while(out>>t)
		{
			v.push_back(t);
		}
	}

private:
	/**
	  * Performs the given operation by first sending it to the
	  * network, waiting for approvement and then performing
	  * it locally.
	 **/
	bool doAndSend(ClusterContainerOperation type, const T &t, const Index &i)
	{
		if(send(type, t, i))
		{
			//Only performing operation after approvement of network
			return perform(type, t, i);
		}
		return false;
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

}; // end class ClusterContainer

/**
  * The ClusterList represents a ClusterContainer
  * which uses an std::list for the internal storage
 **/
template<class T, class Index=unsigned int> using ClusterList = ClusterContainer<Index, T, std::list<T> >;

/**
  * The ClusterList represents a ClusterContainer
  * which uses an std::vector for the internal storage
 **/
template<class T, class Index=unsigned int> using ClusterVector = ClusterContainer<Index, T, std::vector<T> >;

} //end namespace cluster

#endif //CLUSTERCONTAINER_HPP
