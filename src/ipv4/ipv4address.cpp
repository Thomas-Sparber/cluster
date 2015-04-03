/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#include <cluster/ipv4/ipv4address.hpp>
#include <string>
#include <sstream>
#include <ctype.h>
#include <stdlib.h>

using namespace std;
using namespace cluster;

IPv4Address::IPv4Address(const unsigned char uc_a[4]) :
	Address(IPv4Address::encode(uc_a)),
	a()
{
	for(unsigned int i = 0; i < 4; i++)
	{
		this->a[i] = uc_a[i];
	}
}

IPv4Address::IPv4Address(const string &str_address) :
	Address(str_address),
	a()
{
	decode(this->address, a);
}

void IPv4Address::decode(const std::string &address, unsigned char a[4])
{
	std::string s;
	unsigned int counter = 0;
	for(unsigned int i = 0; i < address.length(); i++)
	{
		char ch = address[i];
		if(ch == '.')
		{
			//More than two dots in an IPv4 Address
			//is not allowed
			if(counter > 2)throw AddressException("IP address contains to many numbers");

			a[counter] = (unsigned char)(atoi(s.c_str()));
			counter++;
			s = "";
		}
		else
		{
			if(!isdigit(ch))throw AddressException("IP address contains characters other than digits.");
			s += ch;
		}
	}
	if(counter != 3)throw AddressException("IP address contains not enough numbers");
	a[counter] = (unsigned char)(atoi(s.c_str()));
}

std::string IPv4Address::encode(const unsigned char a[4])
{
	std::stringstream ss;
	ss<<short(a[0])<<"."<<short(a[1])<<"."<<short(a[2])<<"."<<short(a[3]);
	return ss.str();
}

Address* IPv4Address::clone() const
{
	return new IPv4Address(*this);
}

void IPv4Address::increase()
{
	bool success = false;

	//Increase the address from right to left
	for(int counter = 3; counter >= 0; counter--)
	{
		if(counter == 3)
		{
			//254 is the maximum number
			//allowed or the last block
			if(a[counter] < 254)
			{
				a[counter]++;
				success = true;
				break;
			}
			else a[counter] = 1;
		}
		else
		{
			if(a[counter] < 255)
			{
				a[counter]++;
				success = true;
				break;
			}
			else a[counter] = 0;
		}
	}

	//The highest possible Address
	//(255.255.255.254) was reached
	if(!success)
	{
		a[0] = 1;
		a[1] = 0;
		a[2] = 0;
		a[3] = 1;
	}

	address = encode(a);
}

bool IPv4Address::isLoopback() const
{
	return a[0] == 127;
}
