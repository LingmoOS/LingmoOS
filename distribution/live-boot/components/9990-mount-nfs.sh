#!/bin/sh

#set -e

do_nfsmount ()
{
	rc=1

	modprobe -q nfs

	if [ -n "${NFSOPTS}" ]
	then
		NFSOPTS="-o ${NFSOPTS}"
	fi

	log_begin_msg "Trying nfsmount -o nolock -o ro ${NFSOPTS} ${NFSROOT} ${mountpoint}"

	# FIXME: This while loop is an ugly HACK round an nfs bug
	i=0
	while [ "$i" -lt 60 ]
	do
		nfsmount -o nolock -o ro ${NFSOPTS} "${NFSROOT}" "${mountpoint}" && rc=0 && break
		sleep 1
		i="$(($i + 1))"
	done

	return ${rc}
}
