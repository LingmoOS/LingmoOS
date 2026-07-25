cmake_minimum_required(VERSION 3.10)
project(CheckQtVersion)

# Find Qt (you can choose Qt5 or Qt6 depending on your setup)
find_package(Qt6 REQUIRED COMPONENTS Core)

# Get the Qt version
set(QT_VERSION ${Qt6_VERSION})

# Write the Qt version to a file
file(WRITE ${CMAKE_BINARY_DIR}/qt_version.txt "${QT_VERSION}")