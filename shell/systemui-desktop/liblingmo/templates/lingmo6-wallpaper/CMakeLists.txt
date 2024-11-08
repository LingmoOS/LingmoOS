cmake_minimum_required(VERSION 3.16)

project(lingmo-%{APPNAMELC})

find_package(ECM 1.4.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(Lingmo REQUIRED)

# TODO: adapt "org.kde.lingmo" here & elsewhere if needed (see README)
lingmo_install_package(package org.kde.lingmo.%{APPNAMELC} wallpapers wallpaper)
