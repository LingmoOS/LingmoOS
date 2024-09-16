NL="
"
OLDIFS=$IFS

# Check for existing KVM or qemu processes still running, owned by the debian-cd user
check_running () {
    PIDS=$(pidof kvm qemu-system-aarch64 qemu-system-x86_64)
    if [ "$PIDS"x != ""x ]; then
	ps wwwu $PIDS | awk '/^debian-/ {print $2}'
    fi
}

grab_file () {
    REMOTE=$(echo $1 | tr -d \')
    LOCAL="source/$2"
    SIZE=$3
    SHA=$4

    LOG=$(ls -1 *.log)

    if [ ! -f $LOCAL ] ; then
	wget -O $LOCAL $REMOTE -a ${LOG}
	local_size=$(stat -c %s $LOCAL)
	check_alg=${SHA%%:*}
	check_prog="sha"${check_alg##SHA}"sum"
	local_check=$(${check_prog} < $LOCAL | awk '{print $1}')
	if [ ${local_size} != $SIZE ] ; then
	    echo "Size mismatch for $REMOTE" >> ${LOG}
	    SOURCE_ERROR=1
	fi
	if [ ${check_alg}":"${local_check} != $SHA ] ; then
	    echo "Checksums mismatch for $REMOTE" >> ${LOG}
	    echo ${check_alg}":"${local_check}
	    echo $SHA
	    SOURCE_ERROR=1
	fi
    fi
}

now () {
    date -u +%FT%H:%M:%SZ
}

calc_time () {
    local START=$(date -d "$1" +%s)
    local STOP=$(date -d "$2" +%s)
    # Do not worry about more than 1 day
    echo $(date -d @$(($STOP-$START)) +"%-Hh%Mm%Ss" -u)
}

# Log something inside the VM
log () {
    DATE=$(now)
    echo "$DATE $0: $*" > /w/out/log/current
    echo "$*"
    sleep 2
}

# Log something, used outside the VM
run_log () {
    DATE=$(now)
    if [ "$LOGFILE"x != ""x ]; then
	echo "$DATE $@" >> $LOGFILE
    fi
    echo $@
}

checksum_files () {
    PROGS="/home/debian-cd/build/parallel-sums /usr/local/bin/parallel-sums"
    for PROG in $PROGS; do
	if [ -x $PROG ] ; then
	    break
	fi
    done

    case ${CODENAME} in
	# Don't generate MD5 or SHA1 for bullseye and newer
	bullseye|bookworm|trixie|sid)
	    if [ -x $PROG ] ; then
		$PROG \
		    --sha256sum SHA256SUMS \
		    --sha512sum SHA512SUMS debian*.* > /dev/null
	    else
		sha256sum debian*.* > SHA256SUMS
		sha512sum debian*.* > SHA512SUMS
	    fi
	    ;;
	# Continue with MD5 and SHA1 for older releases
	*)
	    if [ -x $PROG ] ; then
		$PROG \
		    --md5sum MD5SUMS \
		    --sha1sum SHA1SUMS \
		    --sha256sum SHA256SUMS \
		    --sha512sum SHA512SUMS debian*.* > /dev/null
	    else
		md5sum debian*.* > MD5SUMS
		sha1sum debian*.* > SHA1SUMS
		sha256sum debian*.* > SHA256SUMS
		sha512sum debian*.* > SHA512SUMS
	    fi
	    ;;
    esac
}

wait_ready () {
    DESIRED="$1"
    READY=0
    STATE=""
    while [ $READY -ne 1 ] ; do
        LAST_STATE="$STATE"
        rsync -a --port=${RSYNC_PORT} ${VM_IP}::live-out/log/current ${CLOG}
        STATE=$(cat ${CLOG})
	if [ "$LAST_STATE"x != "$STATE"x ] ; then
	    run_log "$STATE"
	fi
        grep -q -E "$DESIRED" ${CLOG} && READY=1
        if grep -q -E "ERROR=\S+" ${CLOG} ; then
            CURRENT_ERROR=$(sed -nr 's/.*ERROR\=(\S+).*/\1/p' ${CLOG})
        fi
        sleep 1
    done
    sleep 1
}

sleep_loop () {
    WAIT=$1
    MSG=$2
    FILE=$3

    while [ $WAIT -gt 0 ]; do
       sleep 1
       if [ "$FILE"x != ""x ] && [ -f "$FILE" ]; then
	   run_log "$MSG complete..."
	   break
       fi
       WAIT=$(($WAIT - 1))
       run_log "Waiting $WAIT seconds for $MSG"
    done
}

validate_arg() {
    local VAR=$1
    local VAROPTS="$(echo $2 | tr ':' ' ')"

    for OPT in $VAROPTS; do
	if [ "${!VAR}"x = "$OPT"x ]; then
	    return
	fi
    done
    # else
    echo "$VAR ${!VAR} not supported - use one of $VAROPTS"
    exit 1
}
