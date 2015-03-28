#include <cluster/client.hpp>
#include <cluster/prototypes/communicationsocket.hpp>
#include <cluster/prototypes/protocol.hpp>
#include <cluster/package.hpp>
#include <assert.h>

using namespace std;
using namespace cluster;

Client::Client(const Address &a, const Protocol &p) :
	address(a.clone()),
	protocol(&p)
{}

Client::Client(const Client &c) :
	address(nullptr),
	protocol(c.protocol)
{
	assert(c.address != nullptr);
	address = c.address->clone();
}

Client& Client::operator=(const Client &c)
{
	assert(c.address != nullptr);

	protocol = c.protocol;
	delete address;
	address = c.address->clone();
	return (*this);
}

Client::~Client()
{
	delete address;
}

bool Client::send(const Package &message, Package *out) const
{
	assert(message.getLength() > 0);

	if(CommunicationSocket *s = protocol->createCommunicationSocket(*address))
	{
		bool success = s->send(message) && s->receive(out);
		delete s;
		return success;
	}
	return false;
}

bool Client::operator==(const Client &c)
{
	assert(address != nullptr);
	assert(c.address != nullptr);

	return (*address) == (*c.address);
}

bool Client::operator==(const Address &a)
{
	assert(address != nullptr);

	return (*address) == a;
}
