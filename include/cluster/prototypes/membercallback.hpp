#ifndef MEMBERCALLBACK_HPP
#define MEMBERCALLBACK_HPP

namespace cluster
{

class Address;

class MemberCallback
{

public:
	virtual ~MemberCallback() {}
	virtual void memberOnline(const Address &ip, bool isMaster) = 0;
	virtual void memberOffline(const Address &ip) = 0;

}; // end class MemberCallback

} // end namespace cluster

#endif
