#!/bin/sh

#set -e

do_netmount()
{
	do_netsetup

	if [ "${NFSROOT}" = "auto" ]
	then
		NFSROOT=${ROOTSERVER}:${ROOTPATH}
	fi

	rc=1

	if ( [ -n "${FETCH}" ] || [ -n "${HTTPFS}" ] || [ -n "${FTPFS}" ] )
	then
		do_httpmount
		return $?
	fi

	if [ "${NFSROOT#*:}" = "${NFSROOT}" ] && [ "$NETBOOT" != "cifs" ]
	then
		NFSROOT=${ROOTSERVER}:${NFSROOT}
	fi

	log_begin_msg "Trying netboot from ${NFSROOT}"

	if [ "${NETBOOT}" != "nfs" ] && do_cifsmount
	then
		rc=0
	elif do_nfsmount
	then
		NETBOOT="nfs"
		export NETBOOT
		rc=0
	fi

	log_end_msg
	return ${rc}
}
