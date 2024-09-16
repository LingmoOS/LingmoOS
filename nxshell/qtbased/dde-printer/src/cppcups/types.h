// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TYPES_H
#define TYPES_H
#include <stddef.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <cups/http.h>

#define CUPS_SNMP_PORT		161	/* SNMP well-known port */
#define CUPS_SNMP_MAX_COMMUNITY	512	/* Maximum size of community name */
#define CUPS_SNMP_MAX_OID	128	/* Maximum number of OID numbers */
#define CUPS_SNMP_MAX_STRING	1024	/* Maximum size of string */
#define CUPS_SNMP_MAX_PACKET	1472	/* Maximum size of SNMP packet */
#define CUPS_SNMP_VERSION_1	0	/* SNMPv1 */
#define CUPS_TC_percent				19

enum cups_asn1_e			/**** ASN1 request/object types ****/
{
  CUPS_ASN1_END_OF_CONTENTS = 0x00,	/* End-of-contents */
  CUPS_ASN1_BOOLEAN = 0x01,		/* BOOLEAN */
  CUPS_ASN1_INTEGER = 0x02,		/* INTEGER or ENUMERATION */
  CUPS_ASN1_BIT_STRING = 0x03,		/* BIT STRING */
  CUPS_ASN1_OCTET_STRING = 0x04,	/* OCTET STRING */
  CUPS_ASN1_NULL_VALUE = 0x05,		/* NULL VALUE */
  CUPS_ASN1_OID = 0x06,			/* OBJECT IDENTIFIER */
  CUPS_ASN1_SEQUENCE = 0x30,		/* SEQUENCE */
  CUPS_ASN1_HEX_STRING = 0x40,		/* Binary string aka Hex-STRING */
  CUPS_ASN1_COUNTER = 0x41,		/* 32-bit unsigned aka Counter32 */
  CUPS_ASN1_GAUGE = 0x42,		/* 32-bit unsigned aka Gauge32 */
  CUPS_ASN1_TIMETICKS = 0x43,		/* 32-bit unsigned aka Timeticks32 */
  CUPS_ASN1_GET_REQUEST = 0xa0,		/* GetRequest-PDU */
  CUPS_ASN1_GET_NEXT_REQUEST = 0xa1,	/* GetNextRequest-PDU */
  CUPS_ASN1_GET_RESPONSE = 0xa2		/* GetResponse-PDU */
};

typedef enum cups_asn1_e cups_asn1_t;	/**** ASN1 request/object types ****/

typedef struct cups_snmp_string_s	/**** String value ****/
{
  unsigned char	bytes[CUPS_SNMP_MAX_STRING];
                    /* Bytes in string */
  unsigned	num_bytes;		/* Number of bytes */
} cups_snmp_string_t;

union cups_snmp_value_u			/**** Object value ****/
{
  int		boolean;		/* Boolean value */
  int		integer;		/* Integer value */
  int		counter;		/* Counter value */
  unsigned	gauge;			/* Gauge value */
  unsigned	timeticks;		/* Timeticks  value */
  int		oid[CUPS_SNMP_MAX_OID];	/* OID value */
  cups_snmp_string_t string;		/* String value */
};

typedef struct cups_snmp_s		/**** SNMP data packet ****/
{
  const char	*error;			/* Encode/decode error */
  http_addr_t	address;		/* Source address */
  int		version;		/* Version number */
  char		community[CUPS_SNMP_MAX_COMMUNITY];
                    /* Community name */
  cups_asn1_t	request_type;		/* Request type */
  unsigned	request_id;		/* request-id value */
  int		error_status;		/* error-status value */
  int		error_index;		/* error-index value */
  int		object_name[CUPS_SNMP_MAX_OID];
                    /* object-name value */
  cups_asn1_t	object_type;		/* object-value type */
  union cups_snmp_value_u object_value;		/* object-value value */
} cups_snmp_t;

#endif // TYPES_H
