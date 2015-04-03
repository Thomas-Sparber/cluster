/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef MEMBERCALLBACK_HPP
#define MEMBERCALLBACK_HPP

namespace cluster
{

class Address;

/**
  * A class which inherits from this class
  * can be registered for getting notified
  * whenever a client goes online or offline
 **/
class MemberCallback
{

public:
	/**
	  * Default destructor
	 **/
	virtual ~MemberCallback() {}

	/**
	  * This function is called whenever a new member is online
	 **/
	virtual void memberOnline(const Address &ip, bool isMaster) = 0;

	/**
	  * This function is called whenever a member goes online
	 **/
	virtual void memberOffline(const Address &ip) = 0;

}; // end class MemberCallback

} // end namespace cluster

#endif //MEMBERCALLBACK_HPP
