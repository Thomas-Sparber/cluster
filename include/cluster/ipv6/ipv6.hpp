/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef IPV6_HPP
#define IPV6_HPP

#include <cluster/prototypes/protocol.hpp>
#include <cluster/ipv6/ipv6address.hpp>
#include <cluster/ipv6/ipv6communicationsocket.hpp>
#include <cluster/ipv6/ipv6listenersocket.hpp>
#include <list>

namespace cluster
{

/**
  * This class represents the IPv6 protocol.
  * It is designed in the way that an IPv6
  * protocol binds to a specific port. Therefore
  * it is also possible to create multiple IPv6
  * protocols
 **/
class IPv6 : public Protocol
{

public:
	/**
	  * Constructs an IPv6 Protocol using
	  * the given port. It is also possible
	  * to set the timeout for a connection
	  * to be accepted and the listen backlog
	  * which is the amount of ip packages which
	  * should be rememebred before they are
	  * accepted
	 **/
	IPv6(uint16_t port, unsigned int timeout=10, unsigned int listenBacklog=50);

	/**
	  * Default destructor
	 **/
	virtual ~IPv6();

	/**
	  * Creates an IPv6 ListenerSocket
	 **/
	virtual ListenerSocket* createListenerSocket() const override;

	/**
	  * Creates an IPv6 CommunicationSocket to
	  * communicate with the given Address which must
	  * be an IPv6Address or IPv4Address
	 **/
	virtual CommunicationSocket* createCommunicationSocket(const Address &address) const override;

	/**
	  * Returns the IPv6Addresses of the current
	  * computer
	 **/
	virtual void getAddresses(std::list<Address*> &out) const override;

	/**
	  * Converts the given Address in string representation
	  * to an IPv6Address
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

}; // end class IPv6

} //end namespace cluster

#endif
