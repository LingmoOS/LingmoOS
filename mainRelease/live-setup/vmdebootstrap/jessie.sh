#!/bin/sh

set -e

# define before sourcing common
SUITE='jessie'
SIZE='5G'
# needs a path for arch and task desktop
IMAGE_PATH='/w/out/'
. /w/in/common

/root/vmdebootstrap/vmdebootstrap \
 ${BASE_OPTS} --user 'user/live' \
 --size ${SIZE} \
 --distribution ${SUITE} \
 --customize "/w/in/${SUITE}-hook.sh" \
 --image ${IMAGE_PATH}${SUITE}.img \
 "$@"

# report results and check we have something valid.
ls -l ${IMAGE_PATH}${SUITE}.img.squashfs
md5sum ${IMAGE_PATH}${SUITE}.img.squashfs
unsquashfs -s ${IMAGE_PATH}${SUITE}.img.squashfs
