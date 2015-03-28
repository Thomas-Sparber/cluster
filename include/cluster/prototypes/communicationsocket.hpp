#ifndef COMMUNICATIONSOCKET_HPP
#define COMMUNICATIONSOCKET_HPP

#include "address.hpp"

namespace cluster
{

class Package;

class CommunicationException
{

public:
	CommunicationException(const std::string &str_text) : text(str_text) {}
	std::string text;

}; //end class CommunicationException

class CommunicationSocket
{

public:
	CommunicationSocket(const Address &a) : address(a.clone()) {}

	CommunicationSocket(const CommunicationSocket &c) : address(c.address->clone()) {}

	CommunicationSocket& operator=(const CommunicationSocket &c)
	{
		delete this->address;
		this->address = c.address->clone();
		return (*this);
	}

	virtual ~CommunicationSocket()
	{
		delete address;
	}

	const Address& getAddress()
	{
		return *address;
	}

	virtual bool send(const Package &data) = 0;
	virtual bool receive(Package *out) = 0;

protected:
	const Address *address;

}; // end class ListenerSocket

} // end namespace cluster

#endif
