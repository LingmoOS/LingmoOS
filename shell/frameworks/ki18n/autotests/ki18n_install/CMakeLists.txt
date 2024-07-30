cmake_minimum_required(VERSION 3.16)

project(ki18n_install)

find_package(KF6I18n)

ki18n_install(${CMAKE_CURRENT_SOURCE_DIR}/po)
