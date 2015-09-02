/**
  *
  * (C) Thomas Sparber
  * thomas@sparber.eu
  * 2013-2015
  *
 **/

#ifndef WINDOWSHELPERS_HPP
#define WINDOWSHELPERS_HPP

#include <winsock2.h>
#include <ws2tcpip.h>

inline int inet_pton(int af, const char *src, void *dst)
{
	struct sockaddr_storage ss;
	int size = sizeof(ss);
	ZeroMemory(&ss, sizeof(ss));

	char src_copy[INET6_ADDRSTRLEN+1];
	strncpy(src_copy, src, INET6_ADDRSTRLEN+1);
	src_copy[INET6_ADDRSTRLEN] = 0;

	if(WSAStringToAddress(src_copy, af, nullptr, reinterpret_cast<struct sockaddr*>(&ss), &size) == 0)
	{
		switch(af)
		{
		case AF_INET:
			*(struct in_addr *)dst = reinterpret_cast<struct sockaddr_in&>(ss).sin_addr;
			return 1;
		case AF_INET6:
			*(struct in6_addr *)dst = reinterpret_cast<struct sockaddr_in6&>(ss).sin6_addr;
			return 1;
		}
	}

	return -1;
}

inline int inet_ntop(int af, void *src, char *dst, unsigned long length)
{
	struct sockaddr ss;
	int size = sizeof(ss);
	ZeroMemory(&ss, sizeof(ss));

	switch(af)
	{
	case AF_INET:
		reinterpret_cast<struct sockaddr_in&>(ss).sin_addr = *(struct in_addr *)src;
		break;
	case AF_INET6:
		reinterpret_cast<struct sockaddr_in6&>(ss).sin6_addr = *(struct in6_addr *)src;
		break;
	}

	ss.sa_family = (u_short)af;
	if(WSAAddressToString(reinterpret_cast<struct sockaddr*>(&ss), size, nullptr, dst, &length) == 0)
	{
		return 0;
	}

	return -1;
}

#endif //WINDOWSHELPERS_HPP
