#ifndef CLUSTERCONTAINER_HPP
#define CLUSTERCONTAINER_HPP

#include <cluster/clusterobjectserialized.hpp>
#include <vector>

namespace cluster
{

/**
  * The class ClusterContainer can be parameterized with
  * any type. IMPORTANT: the operator<<(Package, const T&)
  * needs to be overloaded for classes and structures
  * A ClusterContainer is a container of objects which
  * is synchronized across a network. This means, if one object
  * is removed on a node, it is also removed on the other nodes.
  * It is adviceable to use a ClusterMutex to synchronize
  * access to a ClusterContainer.
  * The ClusterContainer inherits ClusterObjectSerialized in
  * order to make sure that one operation is performed after
  * the other and to make sure that a node which joins the
  * network at a later point of the time, contains the same
  * content as the others. ClusterObjectSerialized is also
  * responsible for handling nodes which lose their connection
  * to the network and join it again later.
 **/
template <class T>
class ClusterContainer : public ClusterObjectSerialized
{

private:
	/**
	  * This enum is used to be sent to the other members
	  * of the network to tell them what to do.
	 **/
	enum ContainerAction : unsigned char
	{
		action_add	= 'a',
		action_set	= 's',
		action_erase	= 'e'
	};

public:
	ClusterContainer(ClusterObject *network, unsigned int maxPackagesToRemember=100) :
		ClusterObjectSerialized(network, maxPackagesToRemember),
		v()
	{}

	virtual ~ClusterContainer() {}

	void add(const T &t)
	{
		doAndSend(action_add, t, 0);
	}

	void set(const T &t, unsigned int i)
	{
		doAndSend(action_set, t, i);
	}

	void erase(unsigned int i)
	{
		char hack[sizeof(T)];
		doAndSend(action_erase, *reinterpret_cast<const T*>(hack), i);
	}

	const T& get(unsigned int i) const
	{
		return v[i];
	}

	unsigned int find(const T &t)
	{
		for(unsigned int i = 0; i < v.size(); i++)
		{
			if(v[i] == t)return i;
		}
		return v.size();
	}

	unsigned int size() const
	{
		return v.size();
	}

	virtual std::string getType() const
	{
		return "Clustercontainer";
	}

protected:
	virtual bool received(const Address &ip, const Package &message, Package &answer, Package &send)
	{
		if(ClusterObjectSerialized::received(ip, message, answer, send))return true;

		bool success = true;
		ContainerAction type;
		T t;
		unsigned int i;
		while(message>>type)
		{
			if(!(message>>t)){ success = false; break; }
			if(!(message>>i)){ success = false; break; }
			success = perform(type, t, i) && success;
		}
		return success;
	}

private:
	void doAndSend(ContainerAction type, const T &t, unsigned int i)
	{
		send(type, t, i);
		perform(type, t, i);
	}

	bool perform(ContainerAction type, const T &t, unsigned int i)
	{
		switch(type)
		{
		case action_add:
			v.push_back(t);
			return true;
		case action_set:
			v[i] = t;
			return true;
		case action_erase:
			v.erase(v.begin()+i);
			return true;
		default:
			return false;
		}
	}

private:
	std::vector<T> v;

}; // end class ClusterContainer

} //end namespace cluster

#endif //CLUSTERCONTAINER_HPP
