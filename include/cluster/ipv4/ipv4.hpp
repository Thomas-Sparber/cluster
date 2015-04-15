/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef IPV4_HPP
#define IPV4_HPP

#include <cluster/prototypes/protocol.hpp>
#include <cluster/ipv4/ipv4address.hpp>
#include <cluster/ipv4/ipv4communicationsocket.hpp>
#include <cluster/ipv4/ipv4listenersocket.hpp>
#include <list>

namespace cluster
{

/**
  * This class represents the IPv4 protocol.
  * It is designed in the way that an IPv4
  * protocol binds to a specific port. Therefore
  * it is also possible to create multiple IPv4
  * protocols
 **/
class IPv4 : public Protocol
{

public:
	/**
	  * Constructs an IPv4 Protocol using
	  * the given port. It is also possible
	  * to set the timeout for a connection
	  * to be accepted and the listen backlog
	  * which is the amount of ip packages which
	  * should be rememebred before they are
	  * accepted
	 **/
	IPv4(uint16_t port, unsigned int timeout=10, unsigned int listenBacklog=50);

	/**
	  * Default destructor
	 **/
	virtual ~IPv4();

	/**
	  * Creates an IPv4 ListenerSocket
	 **/
	virtual ListenerSocket* createListenerSocket() const override;

	/**
	  * Creates an IPv4 CommunicationSocket to
	  * communicate with the given Address which must
	  * be an IPv4Address
	 **/
	virtual CommunicationSocket* createCommunicationSocket(const Address &address) const override;

	/**
	  * Returns the IPv4Addresses of the current
	  * computer
	 **/
	virtual void getAddresses(std::list<Address*> &out) const override;

	/**
	  * Converts the given Address in string representation
	  * to an IPv4Address
	 **/
	virtual Address* decodeAddress(const std::string &address) const override;

	/**
	  * Gets the timeout for establishing a connection
	 **/
	unsigned int getTimeout() const
	{
		return timeout;
	}

	/**
	  * Sets the timeout for establishing a connection
	 **/
	void setTimeout(unsigned int ui_timeout)
	{
		this->timeout = ui_timeout;
	}

	/**
	  * Gets the amount of listening packages which
	  * are remembered before they are processed
	 **/
	unsigned int getListenBacklog() const
	{
		return listenBacklog;
	}

	/**
	  * Sets the amount of listening packages which
	  * are remembered before they are processed
	 **/
	void setListenBacklog(unsigned int ui_listenBacklog)
	{
		this->listenBacklog = ui_listenBacklog;
	}

private:
	/**
	  * The port which is used for communication
	 **/
	uint16_t port;

	/**
	  * The timeout for establishing a connection
	 **/
	unsigned int timeout;

	/**
	  * The amount of listening packages which
	  * are remembered before they are processed
	 **/
	unsigned int listenBacklog;

}; // end class IPv4

} //end namespace cluster

#endif
