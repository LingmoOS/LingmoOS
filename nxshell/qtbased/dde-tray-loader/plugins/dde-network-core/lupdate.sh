#!/bin/bash
export QT_SELECT=qt5
cd ./dock-network-plugin
sh ./lupdate.sh
cd ../
cd ./src
sh ./lupdate.sh
cd ../
cd ./network-service-plugin
sh ./lupdate.sh
cd ../

tx push -s --branch m20
