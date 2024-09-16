#!/bin/bash

. /usr/lib/deepin-graphics-driver-manager/common.sh

REMOVEOLD=$1
INSTALL=$2

OLD_DRIVER=$3
NEW_DRIVER=$4

USER_LANG=$5
EXIT_TEST=$6


#check network
check_network || error_exit "The network is not working, please check the network connection" ${NETWORK_CONNECTION_ERROR}

# prepare working directory
mkdir -p $WORKING_DIR_G || error_exit "mkdir $WORKING_DIR_G failed" ${COMMON_ERROR}

# copy remove/install scripts to working dir
cp -f $REMOVEOLD $REMOVE_OLD_G || error_exit "copy $REMOVEOLD failed" ${COMMON_ERROR}

cp -f $INSTALL $INSTALL_NEW_G || error_exit "copy $INSTALL failed" ${COMMON_ERROR}

chmod +x $REMOVE_OLD_G
chmod +x $INSTALL_NEW_G

overlayroot_enable || error_exit "overlayroot-enable failed" ${COMMON_ERROR}

# generate config file
echo "[General]
lang=$USER_LANG
exit_test=$EXIT_TEST
old_driver=$OLD_DRIVER
new_driver=$NEW_DRIVER
gltest-success=false
exit_code=0
notified=false
" | tee $CONFIG_FILE_G

chmod 666 $CONFIG_FILE_G

exit 0
