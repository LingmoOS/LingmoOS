#!/bin/bash

. /usr/lib/deepin-graphics-driver-manager/common.sh


#check the network
check_network || error_exit "The network is not working, please check the network connection" ${NETWORK_CONNECTION_ERROR}

#apt-get update
apt_update || error_exit "Execute apt update failed" ${APT_UPDATE_ERROR}

#remove/install drivers
$REMOVE_OLD_G || error_exit "Remove old driver failed" ${PURGE_PACKAGE_ERROR}
$INSTALL_NEW_G || error_exit "Install new driver failed" ${INSTALL_PACKAGE_ERROR}

cleanWorking

reboot
