
/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2018.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU Lesser General Public License as published   *
* by the Free Software Foundation, either version 3 or (at your option)   *
* any later version. This program is distributed without any warranty.    *
* See the files COPYING.lgpl-v3 and COPYING.gpl-v3 for details.           *
\*************************************************************************/

//=============================================================================

#ifndef INET_SOCKETS_H
#define INET_SOCKETS_H

//=============================================================================

#define   _GNU_SOURCE

//=============================================================================

#include <sys/socket.h>
#include <netdb.h>

//=============================================================================

int inetConnect(const char *host, const char *service, int type);
int inetListen(const char *service, int backlog, socklen_t *const addrLen);
int inetBind(const char *service, int type, socklen_t *const addrlen);
char *inetAddressStr(const struct sockaddr *addr, socklen_t addrlen, 
			char *addrStr, int addrStrLen);

#define			IN_ADDR_STR_LEN			4096
// should be > NI_MAXHOST + NI_MAXSERV + 4

//=============================================================================

#endif // INET_SOCKETS_H

//=============================================================================