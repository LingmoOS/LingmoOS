#!/bin/sh

set -e

# define before sourcing common
SUITE='jessie'
SIZE='5G'
ARCH='arm64'
BINFMT='/usr/bin/qemu-aarch64-static'

# needs a path for arch and task desktop
IMAGE_PATH='/w/out/'
. /w/in/common

/root/vmdebootstrap/vmdebootstrap \
 ${BASE_OPTS} --user 'user/live' \
 --size ${SIZE} \
 --arch ${ARCH} \
 --foreign ${BINFMT} \
 --no-extlinux \
 --grub \
 --package dosfstools \
 --distribution ${SUITE} \
 --customize "/w/in/${SUITE}-${ARCH}-hook.sh" \
 --image ${IMAGE_PATH}${SUITE}-${ARCH}.img \
 "$@"

# report results and check we have something valid.
ls -l ${IMAGE_PATH}${SUITE}-${ARCH}.img.squashfs
md5sum ${IMAGE_PATH}${SUITE}-${ARCH}.img.squashfs
unsquashfs -s ${IMAGE_PATH}${SUITE}-${ARCH}.img.squashfs
