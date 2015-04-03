/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef CLIENT_HPP
#define CLIENT_HPP

namespace cluster
{

class Package;
class Address;
class Protocol;

/**
  * This class is responsible for communicating
  * with other members. The ceonnection to the
  * remote host is established each time the send
  * function is called.
 **/
class Client
{

public:
	/**
	  * Constructus a client to communicate
	  * with the given address using the given protocol
	 **/
	Client(const Address &address, const Protocol &protocol);

	/**
	  * Copy constructor
	 **/
	Client(const Client &c);

	/**
	  * Assignment operator
	 **/
	Client& operator=(const Client &c);

	/**
	  * Default destructor
	 **/
	~Client();

	/**
	  * Compares if the current and the given
	  * client have the same target address
	 **/
	bool operator==(const Client &c);

	/**
	  * Compares if the current target address
	  * is the same as the given address
	 **/
	bool operator==(const Address &a);

	/**
	  * Sends the given package and stores
	  * the answer into out if set.
	 **/
	bool send(const Package &message, Package *out=nullptr) const;

	/**
	  * Returns the target address
	 **/
	const Address& getAddress() const
	{
		return (*address);
	}

private:
	/**
	  * The target address of the client
	 **/
	Address *address;

	/**
	  * The protocol which is used to communicate
	  * with the target address
	 **/
	const Protocol *protocol;

}; //end class Client

} //end namespace cluster

#endif //CLIENT_HPP
