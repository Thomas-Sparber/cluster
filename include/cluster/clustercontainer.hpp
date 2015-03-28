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
template <class T>
class ClusterContainer : public ClusterObjectSerialized
{

public:
	ClusterContainer(ClusterObject *network, unsigned int ui_maxPackagesToRemember=100) :
		ClusterObjectSerialized(network, ui_maxPackagesToRemember),
		v()
	{}

	virtual ~ClusterContainer() {}

	bool add(const T &t)
	{
		return doAndSend(action_add, t, 0);
	}

	bool set(const T &t, unsigned int i)
	{
		return doAndSend(action_set, t, i);
	}

	bool erase(unsigned int i)
	{
		char hack[sizeof(T)];
		return doAndSend(action_erase, *reinterpret_cast<const T*>(hack), i);
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
	virtual bool received(const Address &ip, const Package &message, Package &answer, Package &to_send)
	{
		if(ClusterObjectSerialized::received(ip, message, answer, to_send))return true;

		bool success = true;
		unsigned char type;
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

	virtual bool perform(const Package &message)
	{
		bool success = true;

		unsigned char previousType;
		T previousT;
		unsigned int previousI;

		unsigned char type;
		T t;
		unsigned int i;

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
				if(previousType != type){success = false; std::cout<<"Different types: "<<previousType<<","<<type<<std::endl;}
				if(previousI != i){success = false; std::cout<<"Different i's: "<<previousI<<","<<i<<std::endl;}
				if(previousT != t){success = false; std::cout<<"Different t's: "<<previousT<<","<<t<<std::endl;}
			}

			previousType = type;
			previousI = i;
			previousT = t;
		}

		return success;
	}

	virtual void getRebuildPackage(Package &out)
	{
		for(unsigned int i = 0; i < v.size(); ++i)
		{
			T &t = v[i];
			out<<t;
		}
	}

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
	bool doAndSend(unsigned char type, const T &t, unsigned int i)
	{
		if(send(type, t, i))
		{
			perform(type, t, i);
			return true;
		}
		return false;
	}

	bool perform(unsigned char type, const T &t, unsigned int i)
	{
		switch(type)
		{
		case action_add:
std::cout<<"\t"<<t<<std::endl;
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

	const static unsigned char action_add = 'a';
	const static unsigned char action_set = 's';
	const static unsigned char action_erase = 'e';

}; // end class ClusterContainer

} //end namespace cluster

#endif //CLUSTERCONTAINER_HPP
