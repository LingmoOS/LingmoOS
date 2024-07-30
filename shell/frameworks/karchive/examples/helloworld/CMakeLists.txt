# For more information see the CMake documentation:
# https://cmake.org/documentation/
# https://community.kde.org/Guidelines_and_HOWTOs/CMake

# Project name
project(HelloWorld)

# Look for the KArchive module
find_package(KF6Archive ${KF_VERSION} REQUIRED)

find_package(Qt6Core REQUIRED)

add_executable(helloworld main.cpp)

# Link our executable with the KArchive library
target_link_libraries(helloworld KF6::Archive Qt6::Core)
