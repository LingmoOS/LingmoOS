// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SNMP_H
#define SNMP_H
#include "types.h"

#include <cups/file.h>

typedef void (*cups_snmp_cb_t)(cups_snmp_t *packet, void *data);

#ifdef __cplusplus
extern "C"
{
#endif

int *_cupsSNMPCopyOID(int *dst, const int *src, int dstsize);
char *_cupsSNMPDefaultCommunity(void);
int _cupsSNMPIsOID(cups_snmp_t *packet, const int *oid);
int _cupsSNMPIsOIDPrefixed(cups_snmp_t *packet, const int   *prefix);
char *_cupsSNMPOIDToString(const int *src, char *dst, size_t dstsize);
int *_cupsSNMPStringToOID(const char *src, int *dst, int dstsize);

int _cupsSNMPOpen(int family);
cups_snmp_t *_cupsSNMPRead(int fd, cups_snmp_t *packet, double timeout);


int                 /* O - Number of OIDs found or -1 on error */
_cupsSNMPWalk(int            fd,    /* I - SNMP socket */
              http_addr_t    *address,  /* I - Address to query */
              int            version,   /* I - SNMP version */
              const char     *community,/* I - Community name */
              const int      *prefix,   /* I - OID prefix */
              double         timeout,   /* I - Timeout for each response in seconds */
              cups_snmp_cb_t cb,    /* I - Function to call for each response */
              void           *data);    /* I - User data pointer that is passed to the callback function */


int                 /* O - 1 on success, 0 on error */
_cupsSNMPWrite(
    int            fd,          /* I - SNMP socket */
    http_addr_t    *address,        /* I - Address to send to */
    int            version,     /* I - SNMP version */
    const char     *community,      /* I - Community name */
    cups_asn1_t    request_type,    /* I - Request type */
    const unsigned request_id,      /* I - Request ID */
    const int      *oid);       /* I - OID */

void _cupsSNMPClose(int fd);
http_addrlist_t *get_interface_addresses(const char *ifname);

#ifdef __cplusplus
}
#endif

#endif // SNMP_H
