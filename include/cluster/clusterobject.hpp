/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef CLUSTEROBJECT_HPP
#define CLUSTEROBJECT_HPP

#include <cluster/package.hpp>
#include <cluster/prototypes/address.hpp>
#include <iostream>
#include <assert.h>

namespace cluster
{

class MemberCallback;

/**
  * This enum defines the actions of the
  * ClusterObject
 **/
enum class ClusterObjectOperation : unsigned char
{
	/**
	  * Indicates that the message is intended
	  * for a child object
	 **/
	current = 'c',

	/**
	  * Indicates that the message is intended
	  * for the current object
	 **/
	child = 's'
};

/**
  * This function is overloaded from the Package class
  * to retrieve a ClusterObjectOperation from a Package
 **/
template <>
inline bool operator>>(const Package &p, ClusterObjectOperation &t)
{
	return p.getAndNext(reinterpret_cast<unsigned char&>(t));
}


/**
  * This function is overloaded from the Package class
  * to insert a ClusterObjectOperation into a Package
 **/
template <>
inline void operator<<(Package &p, const ClusterObjectOperation &t)
{
	p.append(reinterpret_cast<const unsigned char&>(t));
}

/**
  * This class is the base class for all objects
  * of a cluster network. It provides basic functions
  * for sending, receiving and asking packages. It also
  * includes methods to register callbacks.
 **/
class ClusterObject
{

public:
	/**
	  * The constructor adds the Clusterobject to the given network
	 **/
	ClusterObject(ClusterObject *network) :
		child(nullptr),
		parent(network ? network->addChild(this) : nullptr)
	{}

	/**
	  * The copy constructor adds the ClusterObject to the
	  * network given by the other ClusterObject
	 **/
	ClusterObject(ClusterObject &o) :
		child(nullptr),
		parent(o.addChild(this))
	{}

	/**
	  * The assignment operator removes the current object
	  * from the current network and adds it to the new one
	  * given by the ClusterObject o
	 **/
	ClusterObject& operator=(ClusterObject &o)
	{
                //Removing this from the current network and adding it to the new one
                removeChild(this);
                this->parent = o.addChild(this);

                return (*this);
	}

	/**
	  * The destructor removes the ClusterObject from the network
	 **/
	virtual ~ClusterObject()
	{
		//The current Clusterobject just needs to be removed from the list
		removeChild(this);
	}

	/**
	  * Adds the givel ClusterObject as child to the network
	 **/
	ClusterObject* addChild(ClusterObject *new_child)
	{
		//Add the child to the last position of the list
		if(this->child)return this->child->addChild(new_child);
		else this->child = new_child;

		return this;
	}

	/**
	  * Removes the given child from the network:
	  * Asks the top parent to remove the child
	 **/
	virtual void removeChild(const ClusterObject *o)
	{
		if(!o)return;

		//Go back to the first parent because it "sees" all child elements
		if(parent)parent->removeChild(o);
		else
		{
			//If no child exists nothing can be removed
			if(!child)return;

			//Check if child is first element
			if(child == o)
			{
				if(child->child)child->child->parent = this;
				child = child->child;
				return;
			}
			
			//Iterate over all children and remove object if found
			ClusterObject *iterator = child;
			while(iterator->child)
			{
				if(iterator->child == o)
				{
					if(iterator->child->child)iterator->child->child->parent = iterator;
					iterator->child = iterator->child->child;
					return;
				}
				iterator = iterator->child;
			}
		}
	}

	/**
	  * Prints the whole structure of the network:
	  * Asks the top parent to print all children's structure
	 **/
	void printWholeStructure()
	{
		//Go to first element and print child structure
		if(parent)parent->printWholeStructure();
		else printChildStructure();
	}

	/**
	  * Prints the child structure of the current ClusterObject
	 **/
	void printChildStructure()
	{
		//Iterates over all children
		ClusterObject *iterator = this;
		unsigned int counter = 1;
		while(iterator)
		{
			//Checks for errors in the network
			if(iterator->parent && iterator->parent->child != iterator)
			{
				std::cout<<"Parent-Child doesn't match"<<std::endl;
				std::cout<<"Parent: "<<iterator->parent->getType()<<std::endl;
				std::cout<<"Child: "<<iterator->getType()<<std::endl;
			}

			//Prints curent element
			std::cout<<(counter++)<<". "<<iterator->getType()<<std::endl;

			//Increases iterator
			iterator = iterator->child;
		}
	}

	/**
	  * Adds a member callback to the network.
	  * The top parent needs to override this
	  * function to actually add the callback
	 **/
	virtual void addMemberCallback(MemberCallback *memberCallback)
	{
		parent->addMemberCallback(memberCallback);
	}

	/**
	  * Removes the member callback from the network.
	  * The top parent needs to override this
	  * function to actually remove the callback
	 **/
	virtual void removeMemberCallback(MemberCallback *memberCallback)
	{
		parent->removeMemberCallback(memberCallback);
	}

	/**
	  * Packs the given object in a package and sends it
	 **/
	template <class A>
	bool send(const A &a, Package *answer=nullptr)
	{
		Package message;
		message<<a;
		return sendPackage(message, answer);
	}

	/**
	  * Packs the given objects in a package and sends them
	 **/
	template <class A, class B>
	bool send(const A &a, const B &b, Package *answer=nullptr)
	{
		Package message;
		message<<a;
		message<<b;
		return sendPackage(message, answer);
	}

	/**
	  * Packs the given objects in a package and sends them
	 **/
	template <class A, class B, class C>
	bool send(const A &a, const B &b, const C &c, Package *answer=nullptr)
	{
		Package message;
		message<<a;
		message<<b;
		message<<c;
		return sendPackage(message, answer);
	}

	/**
	  * Packs the given object in a package and sends it
	  * to the given address only
	 **/
	template <class A>
	bool ask(const Address &ip, const A &a, Package *answer=nullptr)
	{
		Package message;
		message<<a;
		return askPackage(ip, message, answer);
	}

	/**
	  * Packs the given objects in a package and sends them
	  * to the given address only
	 **/
	template <class A, class B>
	bool ask(const Address &ip, const A &a, const B &b, Package *answer=nullptr)
	{
		Package message;
		message<<a;
		message<<b;
		return askPackage(ip, message, answer);
	}

	/**
	  * Packs the given objects in a package and sends them
	  * to the given address only
	 **/
	template <class A, class B, class C>
	bool ask(const Address &ip, const A &a, const B &b, const C &c, Package *answer=nullptr)
	{
		Package message;
		message<<a;
		message<<b;
		message<<c;
		return askPackage(ip, message, answer);
	}

	/**
	  * Returns the type of ClusterObject
	 **/
	virtual std::string getType() const = 0;

protected:
	/**
	  * This function is called internally by the network whenever a package
	  * is received for th current object
	 **/
	virtual bool received(const Address &ip, const Package &message, Package &answer, Package &to_send) = 0;

	/**
	  * This function is called internally by the network whenever a package
	  * is received. This function analyzes whether the message is for
	  * the current object or a child object
	 **/
	bool ClusterObject_received(const Address &ip, const Package &message, Package &answer, Package &to_send)
	{
		bool success = false;
		ClusterObjectOperation t;
		if(!(message>>t))return false;
		switch(t)
		{
		case ClusterObjectOperation::current:
			success = received(ip, message, answer, to_send);
			break;
		case ClusterObjectOperation::child:
			if(this->child)success = child->ClusterObject_received(ip, message, answer, to_send);
			break;
		default:
			success = false;
			break;
		}

		//If something should be sent add child structure
		if(to_send.getLength())
		{
			Package temp;
			temp<<t;
			temp<<to_send;
			to_send = temp;
		}

		return success;
	}

	/**
	  * Adds a signature to the package which indicates that
	  * the package was sent by a child.
	 **/
	Package addChildSignature(const Package &a)
	{
		return addSignature(a, ClusterObjectOperation::child);
	}

	/**
	  * Adds a signature to the package which indicates that
	  * the package was sent by the current object.
	 **/
	Package addCurrentSignature(const Package &a)
	{
		return addSignature(a, ClusterObjectOperation::current);
	}

	/**
	  * Adds the given signature to the package.
	 **/
	Package addSignature(const Package &a, const ClusterObjectOperation type)
	{
		Package message;
		message<<type;
		message<<a;
		return message;
	}

	/**
	  * This function should be called by the current
	  * object to send a message. Then the message
	  * is received for the current object (at the other
	  * side(s))
	 **/
	virtual bool sendPackage(const Package &a, Package *answer)
	{
		return ClusterObject_send(addCurrentSignature(a), answer);
	}

	/**
	  * This function should be called by the current
	  * object to ask a member. Then the message
	  * is received for the current object (at the other
	  * side(s))
	 **/
	virtual bool askPackage(const Address &ip, const Package &a, Package *answer)
	{
		return ClusterObject_ask(ip, addCurrentSignature(a), answer);
	}

	/**
	  * This is the final function which is called
	  * to send a package. The top parent needs to
	  * override this function to actually send the message.
	 **/
	virtual bool ClusterObject_send(const Package &a, Package *answer)
	{
		return parent->ClusterObject_send(addChildSignature(a), answer);
	}

	/**
	  * This is the final function which is called
	  * to ask a member. The top parent needs to
	  * override this function to actually ask the member.
	 **/
	virtual bool ClusterObject_ask(const Address &ip, const Package &a, Package *answer)
	{
		return parent->ClusterObject_ask(ip, addChildSignature(a), answer);
	}

private:
	/**
	  * This is a pointer to the next child object of the network
	 **/
	ClusterObject *child;

	/**
	  * This is a pointer to the parent of the network
	 **/
	ClusterObject *parent;

}; //end class ClusterObject

} // end namespace cluster

#endif //CLUSTEROBJECT_HPP
