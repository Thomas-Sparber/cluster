#ifndef CLUSTEROBJECT_HPP
#define CLUSTEROBJECT_HPP

#include <cluster/package.hpp>
#include <cluster/prototypes/address.hpp>
#include <iostream>
#include <assert.h>

namespace cluster
{

class MemberCallback;

class ClusterObject
{

private:
	enum ClusterObjectMessageType : unsigned char
	{
		message_child = 's',
		message_current = 'c'
	};

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
//		if(child)
//		{
			//I need to store the child in a temp variable.
		        //There could be problems when adding it to the network of
		        //object o when "this" is already part of the network.
		        //Consider this is the last element of the network and child
		        //is added to the network. This means that child is appended
		        //after this. When i set child to nullptr afterwards I remove it.
		        //Complicated? :-)
//		        ClusterObject *temp = child;

		        //The current child is not needed anymore because it
		        //is added to the network
//		        child = nullptr;

		        //The current child needs to be added to the network
		        //Otherwise it would be lost. I don't care about the return
		        //value which is the parent. I don't need it now because I
		        //get it afterwards when I add "this" to the network
//		        o.addChild(temp);
//		}

                //Removing this from the current network and adding it to the new one
                removeChild(this);
                this->parent = o.addChild(this);

                return (*this);
	}

	/**
	  * The destructor removes the ClusterObject from the network2acgi-
	 **/
	virtual ~ClusterObject()
	{
		//The current Clusterobject just needs to be removed from the list
		removeChild(this);
	}

	ClusterObject* addChild(ClusterObject *child)
	{
		//Add the child to the last position of the list
		if(this->child)return this->child->addChild(child);
		else this->child = child;

		return this;
	}

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

	void printWholeStructure()
	{
		//Go to first element and print child structure
		if(parent)parent->printWholeStructure();
		else printChildStructure();
	}

	void printChildStructure()
	{
		ClusterObject *iterator = this;
		unsigned int counter = 1;
		while(iterator)
		{
			if(iterator->parent && iterator->parent->child != iterator)
			{
				std::cout<<"Parent-Child doesn't match"<<std::endl;
				std::cout<<"Parent: "<<iterator->parent->getType()<<std::endl;
				std::cout<<"Child: "<<iterator->getType()<<std::endl;
			}
			std::cout<<(counter++)<<". "<<iterator->getType()<<std::endl;
			iterator = iterator->child;
		}
	}

	virtual void addMemberCallback(MemberCallback *memberCallback)
	{
		parent->addMemberCallback(memberCallback);
	}

	virtual void removeMemberCallback(MemberCallback *memberCallback)
	{
		parent->removeMemberCallback(memberCallback);
	}

	template <class A>
	void send(const A &a, Package *answer=nullptr)
	{
		Package message;
		//message<<message_current;
		message<<a;
		/*ClusterObject_*/sendPackage(message, answer);
	}

	template <class A, class B>
	void send(const A &a, const B &b, Package *answer=nullptr)
	{
		Package message;
		//message<<message_current;
		message<<a;
		message<<b;
		/*ClusterObject_*/sendPackage(message, answer);
	}

	template <class A, class B, class C>
	void send(const A &a, const B &b, const C &c, Package *answer=nullptr)
	{
		Package message;
		//message<<message_current;
		message<<a;
		message<<b;
		message<<c;
		/*ClusterObject_*/sendPackage(message, answer);
	}

	template <class A>
	bool ask(const Address &ip, const A &a, Package *answer=nullptr)
	{
		Package message;
		//message<<message_current;
		message<<a;
		return /*ClusterObject_*/askPackage(ip, message, answer);
	}

	template <class A, class B>
	bool ask(const Address &ip, const A &a, const B &b, Package *answer=nullptr)
	{
		Package message;
		//message<<message_current;
		message<<a;
		message<<b;
		return /*ClusterObject_*/askPackage(ip, message, answer);
	}

	template <class A, class B, class C>
	bool ask(const Address &ip, const A &a, const B &b, const C &c, Package *answer=nullptr)
	{
		Package message;
		//message<<message_current;
		message<<a;
		message<<b;
		message<<c;
		return /*ClusterObject_*/askPackage(ip, message, answer);
	}

	virtual std::string getType() const = 0;

protected:
	virtual bool received(const Address &ip, const Package &message, Package &answer, Package &send) = 0;

	bool ClusterObject_received(const Address &ip, const Package &message, Package &answer, Package &send)
	{
		bool success = false;
		ClusterObjectMessageType t;
		if(!(message>>t))return false;
		switch(t)
		{
		case message_current:
			success = received(ip, message, answer, send);
			break;
		case message_child:
			if(this->child)success = child->ClusterObject_received(ip, message, answer, send);
			break;
		default:
			success = false;
			break;
		}

		//If something should be sent add child structure
		if(send.getLength())
		{
			Package temp;
			temp<<t;
			temp<<send;
			send = temp;
		}

		return success;
	}

	Package addChildSignature(const Package &a)
	{
		return addSignature(a, message_child);
	}

	Package addCurrentSignature(const Package &a)
	{
		return addSignature(a, message_current);
	}

	Package addSignature(const Package &a, ClusterObjectMessageType type)
	{
		Package message;
		message<<type;
		message<<a;
		return message;
	}

	virtual void sendPackage(const Package &a, Package *answer)
	{
		ClusterObject_send(addCurrentSignature(a), answer);
	}

	virtual bool askPackage(const Address &ip, const Package &a, Package *answer)
	{
		return ClusterObject_ask(ip, addCurrentSignature(a), answer);
	}

	virtual void ClusterObject_send(const Package &a, Package *answer)
	{
		parent->ClusterObject_send(addChildSignature(a), answer);
	}

	virtual bool ClusterObject_ask(const Address &ip, const Package &a, Package *answer)
	{
		return parent->ClusterObject_ask(ip, addChildSignature(a), answer);
	}

public:
	ClusterObject *child;
	ClusterObject *parent;

}; //end class ClusterObject

} // end namespace cluster

#endif
