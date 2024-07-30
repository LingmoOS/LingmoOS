cmake_minimum_required(VERSION 3.16)
project(kdoctools_install)

find_package(ECM NO_MODULE)
# required by KF6DocToolsMacros
include(KDEInstallDirs)

find_package(KF6DocTools REQUIRED)

add_subdirectory(../../docs docs)
kdoctools_install(po)
