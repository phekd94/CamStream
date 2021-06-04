
/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2018.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

//=============================================================================

//#define   _GNU_SOURCE 

//=============================================================================

#include "CamStream_InetSockets.h"
#include "CamStream_TLPI.h"

//=============================================================================

#define STR_PREF		"[net]: "

//=============================================================================

// TODO: gai_strerror(ret) !!!

//=============================================================================

int 
inetConnect(const char *host, const char *service, int type)
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sfd, ret;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_addr = NULL;
	hints.ai_canonname = NULL;
	hints.ai_next = NULL;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = type;
	
	hints.ai_flags |= AI_NUMERICHOST; 
	// host == numerical network address (x.x.x.x (ipv4))
	//
	hints.ai_flags |= AI_NUMERICSERV;
	// service is a string containing a numeric port number
	// (for inhibit the invocation of a name resolution service)
	//
	// hints.ai_flags |= AI_CANONNAME;
	// ai_canonname field is official name of the host in 
	// the returned list structures
	// 
	// hints.ai_flags |= AI_ADDRCONFIG;
	// for only one family (v4 or v6)
	// 
	// hints.ai_flags |= AI_V4MAPPED;
	// if specified AF_INET6 not found that return 
	// IPv4-mapped IPv6 addresses in the list
	//
	// hints.ai_flags |= AI_V4MAPPED | AI_ALL;
	// return both IPv6 and IPv4-mapped IPv6 addresses in the list 

	ret = getaddrinfo(host, service, &hints, &result);
	if (ret != 0) {
		printf(STR_PREF "getaddrinfo: %s", gai_strerror(ret));
		if (errno != EAI_SYSTEM) {
			errno = ENOSYS;
		} else {
			// errMsg(STR_PREF "getaddrinfo");
		}
		return -1;
	}

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sfd == -1)
			continue;

		ret = connect(sfd, rp->ai_addr, rp->ai_addrlen);
		if (ret != -1)
			break;

		close(sfd);
	}
	
	freeaddrinfo(result);

	if (rp == NULL) {
		errno = ENOSYS;
		return -1;
	}
	else
		return sfd;
}

//=============================================================================

static int
inetPassiveSocket(const char *service, int type, socklen_t *const addrlen, 
		Boolean doListen, int backlog)
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sfd, ret, optval;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_addr = NULL;
	hints.ai_canonname = NULL;
	hints.ai_next = NULL;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = type;
	hints.ai_flags = AI_PASSIVE; // for NULL in first param (below)

	hints.ai_flags |= AI_NUMERICHOST;
	hints.ai_flags |= AI_NUMERICSERV;

	ret = getaddrinfo(NULL, service, &hints, &result);
	if (ret != 0) {
		printf(STR_PREF "getaddrinfo: %s", gai_strerror(ret));
		if (errno != EAI_SYSTEM) {
			errno = ENOSYS;
		} else {
			// errMsg(STR_PREF "getaddrinfo");
		}
		return -1;
	}

	optval = 1;
	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sfd == -1)
			continue;

		if (doListen) {
			if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, 
					sizeof(optval)) == -1) {
				close(sfd);
				freeaddrinfo(result);
				return -1;	
			}
		}

		ret = bind(sfd, rp->ai_addr, rp->ai_addrlen);
		if (ret == 0)
			break;

		close(sfd);
	}

	if (rp != NULL && doListen) {
		ret = listen(sfd, backlog);
		if (ret == -1) {
			close(sfd);
			freeaddrinfo(result);
			return -1;
		}
	}

	if (rp != NULL && addrlen != NULL)
		*addrlen = rp->ai_addrlen;
	
	freeaddrinfo(result);

	if (rp == NULL) {
		errno = ENOSYS;
		return -1;
	}
	else
		return sfd;
}

//=============================================================================

int 
inetListen(const char *service, int backlog, socklen_t *const addrlen)
					// C: ISO 6.7.6.1 --^
{
	return inetPassiveSocket(service, SOCK_STREAM, addrlen, TRUE, backlog);
}

//=============================================================================

int 
inetBind(const char *service, int type, socklen_t *const addrlen)
{
	return inetPassiveSocket(service, type, addrlen, FALSE, 0);
}

//=============================================================================

char *
inetAddressStr(const struct sockaddr *addr, socklen_t addrlen, 
		char *addrStr, int addrStrLen)
{
	char host[NI_MAXHOST], service[NI_MAXSERV];
	int ret;

	ret = getnameinfo(addr, addrlen, host, NI_MAXHOST, 
		service, NI_MAXSERV, 
		NI_NUMERICSERV | NI_NUMERICHOST ); // | NI_NOFQDN);
	if (ret == 0) { 
		snprintf(addrStr, addrStrLen, "(%s, %s)", host, service);
	} else {
		snprintf(addrStr, addrStrLen, "(UNKNOWN)");
		if (ret == EAI_SYSTEM) {
			errMsg(STR_PREF "getnameinfo");
		} else {
			printf(STR_PREF "getnameinfo: %s", gai_strerror(ret));
		}
	}

	return addrStr;
}

//=============================================================================
