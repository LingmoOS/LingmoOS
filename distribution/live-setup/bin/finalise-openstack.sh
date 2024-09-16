#!/bin/bash

SCRIPTS_SOURCE="/home/debian-cd/live/available"
. $SCRIPTS_SOURCE/common.sh

if [ "$DRYRUN"x != ""x ]; then
    echo "Dry-run $0: ARCH $ARCH TYPE $TYPE"
    exit 0
fi

. ~/build.${CODENAME}/settings.sh
. ~/build.${CODENAME}/common.sh

if [ "$RELEASE_BUILD"x = ""x ] ; then
    CONF=${SCRIPTS_SOURCE}/CONF.sh
    OUT=${OUT_BASE}/openstack
else
    CONF=$SCRIPTS_SOURCE/CONF.sh.${RELEASE_BUILD}
    OUT=${OUT_BASE}/.${RELEASE_BUILD}/openstack
fi
. $CONF

my_sync () {
    local LOCAL=$1
    local REMOTE=$2

    SYNC_START=$(now)
    echo "Starting sync of $LOCAL to umu.se at $SYNC_START"
    rsync_to_umu $LOCAL/ $REMOTE/
    # Just for safety, make sure there are no trailing slashes here -
    # will make a mess on the publish step
    REMOTE=$(echo $REMOTE | sed 's,/*$,,')
    publish_at_umu $REMOTE
    SYNC_END=$(now)
    time_spent=$(calc_time $SYNC_START $SYNC_END)
    echo "  Sync of $LOCAL to umu.se; started at $SYNC_START, ended at $SYNC_END (took $time_spent)"
}

# We should have two arch-specific directories here already. Bring
# them together, checksum, etc.
# We should have two arch-specific directories here already. Bring
# them together, checksum, etc.
WD=/home/debian-cd/publish/.live/openstack-$CODENAME
rm -rf $WD
mkdir $WD
cd $WD
mv /home/debian-cd/publish/.live/mount/*-openstack-$CODENAME/out/openstack-$CODENAME/*/* .
rmdir /home/debian-cd/publish/.live/mount/*-openstack-$CODENAME/out/openstack-$CODENAME/*
checksum_files

cp ~/build.${CODENAME}/openstack-header.html HEADER.html

if [ "$RELEASE_BUILD"x = ""x ] ; then
    # Sign weekly images using our testing key
    ~/build.${CODENAME}/sign-images $WD ""

    # Then publish as testing on umu.se
    if [ $(hostname) = casulana ]; then
	my_sync $WD openstack/testing
    fi

else
    # Don't sign a release build, we'll do that manually later.
    if [ $(hostname) = casulana ]; then
	if [ "$SUDO_USER"x != ""x ]; then
	    echo "$SUDO_USER" > $WD/.user
	else
	    echo "Can't identify build user from SUDO_USER! Abort."
	    exit 1
	fi
	my_sync $WD .${RELEASE_BUILD}/openstack
    fi
fi
