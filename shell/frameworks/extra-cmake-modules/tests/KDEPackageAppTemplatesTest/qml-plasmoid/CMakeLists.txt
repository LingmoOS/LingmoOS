cmake_minimum_required(VERSION 3.5)

project(lingmo-%{APPNAMELC})
 
find_package(ECM 1.4.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(KF5Lingmo REQUIRED)

lingmo_install_package(package org.kde.%{APPNAMELC})
