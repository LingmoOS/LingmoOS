/* getifaddrs -- get names and addresses of all network interfaces
   Copyright (C) 2013-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <ifaddrs.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <hurd.h>
#include <hurd/paths.h>
#include <hurd/lookup.h>
#include <hurd/fs.h>

/* Create a linked list of `struct ifaddrs' structures, one for each
   network interface on the host machine.  If successful, store the
   list in *IFAP and return 0.  On errors, return -1 and set `errno'.  */
int
__getifaddrs (struct ifaddrs **ifap)
{
  /* XXX: Hackish.  This assumes pfinet parameter style, and that the same
     pfinet is on /servers/socket/2 and /servers/socket/26.

     To be replaced by something like a netlink protocol, or fix ifreq into
     using sockaddr_storage (but break existing compiled programs using it).  */

  file_t node;
  char *argz = 0, *cur;
  mach_msg_type_number_t argz_len = 0;
  unsigned naddrs;
  const char *ifa_name = NULL;
  char *addr, *cidr_a;
  int cidr;

  node = __file_name_lookup (_SERVERS_SOCKET "/2", 0, 0666);

  if (node == MACH_PORT_NULL)
    return -1;

  __file_get_fs_options (node, &argz, &argz_len);

  __mach_port_deallocate (__mach_task_self (), node);

  /* XXX: Two hardcoded for lo */
  naddrs = 2;

  for (cur = argz; cur < argz + argz_len; cur = cur + strlen (cur) + 1)
    {
      if (!strncmp (cur, "--address=", 10))
	  naddrs++;
      else if (!strncmp (cur, "--address6=", 11))
	  naddrs++;
    }

    {
      struct
      {
	struct ifaddrs ia;
	struct sockaddr_storage addr, netmask, broadaddr;
	char name[IF_NAMESIZE];
      } *storage;
      int i;
      struct sockaddr_in *sin;
      struct sockaddr_in6 *sin6;

      storage = malloc (naddrs * sizeof storage[0]);
      if (storage == NULL)
	{
	  __munmap (argz, argz_len);
	  return -1;
	}

      i = 0;

      /* XXX: Hardcoded lo interface */
      ifa_name = "lo";

      /* 127.0.0.1/8 */
      storage[i].ia.ifa_next = &storage[i + 1].ia;
      storage[i].ia.ifa_name = strncpy (storage[i].name, ifa_name, sizeof (storage[i].name));

      storage[i].ia.ifa_addr = (struct sockaddr *) &storage[i].addr;
      sin = ((struct sockaddr_in *) &storage[i].addr);
      sin->sin_family = AF_INET;
      sin->sin_len = sizeof(*sin);
      sin->sin_port = 0;
      sin->sin_addr.s_addr = htonl (INADDR_LOOPBACK);

      storage[i].ia.ifa_netmask = (struct sockaddr *) &storage[i].netmask;
      sin = ((struct sockaddr_in *) &storage[i].netmask);
      sin->sin_family = AF_INET;
      sin->sin_len = sizeof(*sin);
      sin->sin_port = 0;
      sin->sin_addr.s_addr = htonl (IN_CLASSA_NET);

      storage[i].ia.ifa_broadaddr = (struct sockaddr *) &storage[i].addr;

      storage[i].ia.ifa_flags = IFF_UP | IFF_LOOPBACK | IFF_RUNNING;

      storage[i].ia.ifa_data = NULL; /* Nothing here for now.  */

      i++;

      /* ::1/128 */
      storage[i].ia.ifa_next = &storage[i + 1].ia;
      storage[i].ia.ifa_name = strncpy (storage[i].name, ifa_name, sizeof (storage[i].name));

      storage[i].ia.ifa_addr = (struct sockaddr *) &storage[i].addr;
      sin6 = ((struct sockaddr_in6 *) &storage[i].addr);
      sin6->sin6_family = AF_INET6;
      sin6->sin6_len = sizeof(*sin6);
      sin6->sin6_port = 0;
      sin6->sin6_flowinfo = 0;
      inet_pton (AF_INET6, "::1", &sin6->sin6_addr);
      sin6->sin6_scope_id = 0;

      storage[i].ia.ifa_netmask = (struct sockaddr *) &storage[i].netmask;
      sin6 = ((struct sockaddr_in6 *) &storage[i].netmask);
      sin6->sin6_family = AF_INET6;
      sin6->sin6_len = sizeof(*sin6);
      sin6->sin6_port = 0;
      sin6->sin6_flowinfo = 0;
      inet_pton (AF_INET6, "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff", &sin6->sin6_addr);
      sin6->sin6_scope_id = 0;

      storage[i].ia.ifa_broadaddr = NULL;

      storage[i].ia.ifa_flags = IFF_UP | IFF_LOOPBACK | IFF_RUNNING;

      storage[i].ia.ifa_data = NULL; /* Nothing here for now.  */

      for (cur = argz; cur < argz + argz_len; cur = cur + strlen (cur) + 1)
	{
	  if (!strncmp (cur, "--interface=", 12))
	    {
	      ifa_name = cur + 12;
	      continue;
	    }

	  else if (!strncmp (cur, "--address=", 10))
	    {
	      i++;
	      /* IPv4 address */
	      addr = cur + 10;

	      storage[i].ia.ifa_next = &storage[i + 1].ia;
	      storage[i].ia.ifa_name = strncpy (storage[i].name, ifa_name, sizeof (storage[i].name));

	      storage[i].ia.ifa_addr = (struct sockaddr *) &storage[i].addr;
	      sin = ((struct sockaddr_in *) &storage[i].addr);
	      sin->sin_family = AF_INET;
	      sin->sin_len = sizeof(*sin);
	      sin->sin_port = 0;
	      inet_pton (AF_INET, addr, &sin->sin_addr);

	      storage[i].ia.ifa_netmask = NULL;
	      storage[i].ia.ifa_broadaddr = NULL;

	      storage[i].ia.ifa_flags = IFF_UP | IFF_BROADCAST | IFF_RUNNING | IFF_MULTICAST;
	      storage[i].ia.ifa_data = NULL; /* Nothing here for now.  */
	    }

	  else if (!strncmp (cur, "--netmask=", 10))
	    {
	      /* IPv4 netmask */
	      addr = cur + 10;

	      storage[i].ia.ifa_netmask = (struct sockaddr *) &storage[i].netmask;
	      sin = ((struct sockaddr_in *) &storage[i].netmask);
	      sin->sin_family = AF_INET;
	      sin->sin_len = sizeof(*sin);
	      sin->sin_port = 0;
	      inet_pton (AF_INET, addr, &sin->sin_addr);

	      storage[i].ia.ifa_broadaddr = (struct sockaddr *) &storage[i].broadaddr;
	      sin = ((struct sockaddr_in *) &storage[i].broadaddr);
	      sin->sin_family = AF_INET;
	      sin->sin_len = sizeof(*sin);
	      sin->sin_port = 0;
	      sin->sin_addr.s_addr =
		  ((struct sockaddr_in *) &storage[i].addr)->sin_addr.s_addr
		| ~(((struct sockaddr_in *) &storage[i].netmask)->sin_addr.s_addr);
	    }

	  else if (!strncmp (cur, "--peer=", 7))
	    {
	      /* IPv4 peer */
	      addr = cur + 7;

	      storage[i].ia.ifa_dstaddr = (struct sockaddr *) &storage[i].broadaddr;
	      sin = ((struct sockaddr_in *) &storage[i].broadaddr);
	      sin->sin_family = AF_INET;
	      sin->sin_len = sizeof(*sin);
	      sin->sin_port = 0;
	      inet_pton (AF_INET, addr, &sin->sin_addr);

	      storage[i].ia.ifa_flags &= ~IFF_BROADCAST;
	      storage[i].ia.ifa_flags |= IFF_POINTOPOINT;
	    }

	  else if (!strncmp (cur, "--address6=", 11))
	    {
	      i++;
	      /* IPv6 address */
	      addr = cur + 11;
	      cidr_a = strchr (addr, '/');
	      if (!cidr_a)
		{
		  /* No CIDR length?! Assume 64.  */
		  addr = __strdup (addr);
		  cidr = 64;
		}
	      else
		{
		  addr = __strndup (addr, cidr_a - addr);
		  cidr = atoi (cidr_a + 1);
		}

	      storage[i].ia.ifa_next = &storage[i + 1].ia;
	      storage[i].ia.ifa_name = strncpy (storage[i].name, ifa_name, sizeof (storage[i].name));

	      storage[i].ia.ifa_addr = (struct sockaddr *) &storage[i].addr;
	      sin6 = ((struct sockaddr_in6 *) &storage[i].addr);
	      sin6->sin6_family = AF_INET6;
	      sin6->sin6_len = sizeof(*sin6);
	      sin6->sin6_port = 0;
	      sin6->sin6_flowinfo = 0;
	      inet_pton (AF_INET6, addr, &sin6->sin6_addr);
	      sin6->sin6_scope_id = 0;

	      storage[i].ia.ifa_netmask = (struct sockaddr *) &storage[i].netmask;
	      sin6 = ((struct sockaddr_in6 *) &storage[i].netmask);
	      sin6->sin6_family = AF_INET6;
	      sin6->sin6_len = sizeof(*sin6);
	      sin6->sin6_port = 0;
	      sin6->sin6_flowinfo = 0;
	      sin6->sin6_addr.s6_addr32[0] = htonl (cidr >=  32 ? 0xffffffffUL : cidr <=  0 ? 0 : ~((1UL << ( 32 - cidr)) - 1));
	      sin6->sin6_addr.s6_addr32[1] = htonl (cidr >=  64 ? 0xffffffffUL : cidr <= 32 ? 0 : ~((1UL << ( 64 - cidr)) - 1));
	      sin6->sin6_addr.s6_addr32[2] = htonl (cidr >=  96 ? 0xffffffffUL : cidr <= 64 ? 0 : ~((1UL << ( 96 - cidr)) - 1));
	      sin6->sin6_addr.s6_addr32[3] = htonl (cidr >= 128 ? 0xffffffffUL : cidr <= 96 ? 0 : ~((1UL << (128 - cidr)) - 1));
	      sin6->sin6_scope_id = 0;

	      storage[i].ia.ifa_broadaddr = NULL;
	      storage[i].ia.ifa_flags = IFF_UP | IFF_BROADCAST | IFF_RUNNING | IFF_MULTICAST;
	      storage[i].ia.ifa_data = NULL; /* Nothing here for now.  */
	      free (addr);
	    }

	  else if (!strncmp (cur, "--peer6=", 8))
	    {
	      /* IPv6 peer */
	      addr = cur + 8;

	      storage[i].ia.ifa_dstaddr = (struct sockaddr *) &storage[i].broadaddr;
	      sin6 = ((struct sockaddr_in6 *) &storage[i].broadaddr);
	      sin6->sin6_family = AF_INET6;
	      sin6->sin6_len = sizeof(*sin6);
	      sin6->sin6_port = 0;
	      sin6->sin6_flowinfo = 0;
	      inet_pton (AF_INET6, addr, &sin6->sin6_addr);
	      sin6->sin6_scope_id = 0;

	      storage[i].ia.ifa_flags &= ~IFF_BROADCAST;
	      storage[i].ia.ifa_flags |= IFF_POINTOPOINT;
	    }
	}

      storage[i].ia.ifa_next = NULL;

      *ifap = &storage[0].ia;
    }

  __munmap (argz, argz_len);

  return 0;
}
weak_alias (__getifaddrs, getifaddrs)
libc_hidden_def (__getifaddrs)
#ifndef getifaddrs
libc_hidden_weak (getifaddrs)
#endif

void
__freeifaddrs (struct ifaddrs *ifa)
{
  free (ifa);
}
weak_alias (__freeifaddrs, freeifaddrs)
libc_hidden_def (__freeifaddrs)
libc_hidden_weak (freeifaddrs)
