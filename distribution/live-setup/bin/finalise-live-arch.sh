#!/bin/bash

SCRIPTS_SOURCE="/home/debian-cd/live/available"
. $SCRIPTS_SOURCE/common.sh

while getopts ":c:a:t:B:" o; do
    case "${o}" in
	c)
	    VMCONFIG=${OPTARG}
	    ;;
        a)
            ARCH=${OPTARG}
            ;;
        t)
            TYPE=${OPTARG}
            ;;
	B)
	    # Easier to drive from the Makefile - accept the full
	    # arch/type name (e.g. "FL_amd64_F") and split it up
	    # here
	    LONG_NAME=${OPTARG}
	    case $LONG_NAME in
		*_F)
		    TYPE=non-free;;
		*)
		    TYPE=free;;
	    esac
	    ARCH=$(echo $LONG_NAME | awk -F "_" '{print $2}')
	    ;;
        *)
	    echo "Unrecognised arg ${o}"
            exit 1
            ;;
    esac
done
shift $((OPTIND-1))

if [ "$DRYRUN"x != ""x ]; then
    echo "Dry-run $0: ARCH $ARCH TYPE $TYPE"
    exit 0
fi

. ~/build.${CODENAME}/settings.sh
. ~/build.${CODENAME}/common.sh

if [ "$RELEASE_BUILD"x = ""x ] ; then
    CONF=${SCRIPTS_SOURCE}/CONF.sh
else
    RELEASE_OUT=${OUT_BASE}/.${RELEASE_BUILD}
    CONF=$SCRIPTS_SOURCE/CONF.sh.${RELEASE_BUILD}
fi
. $CONF

export WEEKLY_LIVE=${OUT_BASE}/weekly-live-builds
export NF_WEEKLY_LIVE=${CD_I_F}/weekly-live-builds

validate_arg ARCH "amd64:i386:source"
validate_arg TYPE "free:non-free"
if [ -f "$VMCONFIG" ]; then
    . $VMCONFIG
else
    echo "$0: Can't load config from $VMCONFIG, aborting"
    exit 1
fi

if [ ${TYPE} = "free" ] ; then
    typestem=""
else
    typestem="+nonfree"
fi

WD=$OUTDIR/out/$TYPE/$ARCH
cd $WD
if [ -d iso-hybrid.tmp ]; then
    mv iso-hybrid.tmp iso-hybrid
    cd iso-hybrid && checksum_files && cd $WD
fi
if [ -d tar.tmp ]; then
    mv tar.tmp tar
    cd tar && checksum_files && cd $WD
fi

my_sync () {
    local LOCAL=$1
    local REMOTE=$2

    if [ "$NOSYNC"x = ""x ] ; then
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
    else
	echo "NOSYNC set, so not syncing to umu.se"
    fi
}

if [ "$RELEASE_BUILD"x = ""x ] ; then
    # Sign weekly images using our testing key
    ~/build.${CODENAME}/sign-images $WD ""

    # Slightly different stuff for free and non-free images now
    if [ $TYPE = "non-free" ]; then
	~/build.${CODENAME}/generate_headers \
		~/build.${CODENAME}/live-header.html.in \
		$WD $ARCH firmware
	WL_OUT=${NF_WEEKLY_LIVE}
	mkdir -p ${NF_WEEKLY_LIVE}
	rm -rf ${NF_WEEKLY_LIVE}/$ARCH
	mv $OUTDIR/out/$TYPE/$ARCH ${NF_WEEKLY_LIVE}/
	# If we're on casulana, push things to umu.se here
	if [ $(hostname) = casulana ]; then
	    my_sync ${NF_WEEKLY_LIVE}/$ARCH unofficial/non-free/images-including-firmware/weekly-live-builds/$ARCH
	fi
    else # free
	~/build.${CODENAME}/generate_headers \
		~/build.${CODENAME}/live-header.html.in \
		$WD $ARCH ""
	mkdir -p ${WEEKLY_LIVE}
	rm -rf ${WEEKLY_LIVE}/$ARCH
	mv $OUTDIR/out/$TYPE/$ARCH ${WEEKLY_LIVE}
	# If we're on casulana, push things to umu.se here
	if [ $(hostname) = casulana ]; then
	    my_sync ${WEEKLY_LIVE}/$ARCH weekly-live-builds/$ARCH
	fi
    fi

else # RELEASE_BUILD is set

    cd $WD/..
    # We make torrent files for binaries on release builds
    if [ $ARCH != source ]; then
	find $ARCH -name '*.iso' | xargs ~/build.${CODENAME}/mktorrent $ARCH
	cd $WD/bt-hybrid
	checksum_files
	# Also include the checksums for the .iso files in the SUMS
	# files in the torrent dir
	for file in *SUMS; do
	    grep .iso$ ../iso-hybrid/$file >> $file
	done
	cd $WD/..
    fi
    
    if [ $TYPE = "non-free" ]; then
	~/build.${CODENAME}/generate_headers \
		~/build.${CODENAME}/live-header.html.in \
		$WD $ARCH firmware
    else # free
	~/build.${CODENAME}/generate_headers \
		~/build.${CODENAME}/live-header.html.in \
		$WD $ARCH ""
    fi

    mkdir -p ${RELEASE_OUT}/live-$TYPE
    rm -rf ${RELEASE_OUT}/live-$TYPE/$ARCH
    mv $OUTDIR/out/$TYPE/$ARCH ${RELEASE_OUT}/live-$TYPE
    # If we're on casulana, push things to umu here
    if [ $(hostname) = casulana ]; then
	my_sync ${RELEASE_OUT}/live-$TYPE/$ARCH .${RELEASE_BUILD}/live-$TYPE/$ARCH
    fi
fi

