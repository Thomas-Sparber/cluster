#ifndef CLIENT_HPP
#define CLIENT_HPP

namespace cluster
{

class Package;
class Address;
class Protocol;

class Client
{

public:
	Client(const Address &address, const Protocol &protocol);
	Client(const Client &c);
	Client& operator=(const Client &c);
	~Client();
	bool operator==(const Client &c);
	bool operator==(const Address &a);
	bool send(const Package &message, Package *out=nullptr) const;
	const Address& getAddress() const
	{
		return (*address);
	}

private:
	Address *address;
	const Protocol *protocol;

}; //end class Client

} //end namespace cluster

#endif //CLIENT_HPP
