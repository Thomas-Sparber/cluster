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
#include <cluster/answerpackage.hpp>
#include <string>

namespace cluster
{

class MemberCallback;
class Address;

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
	ClusterObject(ClusterObject *network);

	/**
	  * The copy constructor adds the ClusterObject to the
	  * network given by the other ClusterObject
	 **/
	ClusterObject(ClusterObject &o);

	/**
	  * The assignment operator removes the current object
	  * from the current network and adds it to the new one
	  * given by the ClusterObject o
	 **/
	ClusterObject& operator=(ClusterObject &o);

	/**
	  * The destructor removes the ClusterObject from the network
	 **/
	virtual ~ClusterObject();

	/**
	  * Adds the givel ClusterObject as child to the network
	 **/
	ClusterObject* addChild(ClusterObject *new_child);

	/**
	  * Removes the given child from the network:
	  * Asks the top parent to remove the child
	 **/
	void removeChild(const ClusterObject *o);

	/**
	  * Returns the whole structure of the network:
	  * Asks the top parent to get all children's structure
	 **/
	std::string getWholeStructure() const;

	/**
	  * Returns the child structure of the current ClusterObject
	 **/
	std::string getChildStructure() const;

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
	bool send(const A &a, AnswerPackage *answer)
	{
		Package message;
		message<<a;
		return sendPackage(message, answer);
	}

	/**
	  * Packs the given objects in a package and sends them
	 **/
	template <class A, class B>
	bool send(const A &a, const B &b, AnswerPackage *answer)
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
	bool send(const A &a, const B &b, const C &c, AnswerPackage *answer)
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
	bool ask(const Address &ip, const A &a, Package *answer)
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
	bool ask(const Address &ip, const A &a, const B &b, Package *answer)
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
	bool ask(const Address &ip, const A &a, const B &b, const C &c, Package *answer)
	{
		Package message;
		message<<a;
		message<<b;
		message<<c;
		return askPackage(ip, message, answer);
	}

	/**
	  * This function should be called by the current
	  * object to send a message. Then the message
	  * is received for the current object (at the other
	  * side(s))
	 **/
	virtual bool sendPackage(const Package &a, AnswerPackage *answer)
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
	bool ClusterObject_received(const Address &ip, const Package &message, Package &answer, Package &to_send);

	/**
	  * Adds a signature to the package which indicates that
	  * the package was sent by a child.
	 **/
	Package addChildSignature(const Package &a);

	/**
	  * Adds a signature to the package which indicates that
	  * the package was sent by the current object.
	 **/
	Package addCurrentSignature(const Package &a);

	/**
	  * This is the final function which is called
	  * to send a package. The top parent needs to
	  * override this function to actually send the message.
	 **/
	virtual bool ClusterObject_send(const Package &a, AnswerPackage *answer)
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
