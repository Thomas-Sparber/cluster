/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#include <cluster/clusterobject.hpp>
#include <cluster/prototypes/address.hpp>
#include <iostream>
#include <sstream>

using namespace std;
using namespace cluster;

namespace cluster
{
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
		return p>>reinterpret_cast<unsigned char&>(t);
	}


	/**
	  * This function is overloaded from the Package class
	  * to insert a ClusterObjectOperation into a Package
	 **/
	template <>
	inline void operator<<(Package &p, const ClusterObjectOperation &t)
	{
		p<<reinterpret_cast<const unsigned char&>(t);
	}


	/**
	  * Adds the given signature to the package.
	 **/
	inline Package addSignature(const Package &a, ClusterObjectOperation type)
	{
		Package message;
		message<<type;
		message<<a;
		return message;
	}

} //end namespace cluster

ClusterObject::ClusterObject(ClusterObject *network) :
	child(nullptr),
	parent(network ? network->addChild(this) : nullptr)
{}

ClusterObject::ClusterObject(ClusterObject &o) :
	child(nullptr),
	parent(o.addChild(this))
{}

ClusterObject& ClusterObject::operator=(ClusterObject &o)
{
            //Removing this from the current network and adding it to the new one
            removeChild(this);
            this->parent = o.addChild(this);

            return (*this);
}

ClusterObject::~ClusterObject()
{
	//The current Clusterobject just needs to be removed from the list
	removeChild(this);
}

ClusterObject* ClusterObject::addChild(ClusterObject *new_child)
{
	//Add the child to the last position of the list
	if(this->child)return this->child->addChild(new_child);
	else this->child = new_child;

	return this;
}

void ClusterObject::removeChild(const ClusterObject *o)
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

string ClusterObject::getWholeStructure() const
{
	//Go to first element and print child structure
	if(parent)return parent->getWholeStructure();
	else return getChildStructure();
}

string ClusterObject::getChildStructure() const
{
	std::stringstream ss;

	//Iterates over all children
	const ClusterObject *iterator = this;
	unsigned int counter = 1;
	while(iterator)
	{
		//Checks for errors in the network
		if(iterator->parent && iterator->parent->child != iterator)
		{
			ss<<"Parent-Child doesn't match"<<std::endl;
			ss<<"Parent: "<<iterator->parent->getType()<<std::endl;
			ss<<"Child: "<<iterator->getType()<<std::endl;
		}

		//Prints curent element
		ss<<(counter++)<<". "<<iterator->getType()<<std::endl;

		//Increases iterator
		iterator = iterator->child;
	}

	return ss.str();
}

bool ClusterObject::ClusterObject_received(const Address &ip, const Package &message, Package &answer, Package &to_send)
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

Package ClusterObject::addChildSignature(const Package &a)
{
	return addSignature(a, ClusterObjectOperation::child);
}

Package ClusterObject::addCurrentSignature(const Package &a)
{
	return addSignature(a, ClusterObjectOperation::current);
}
