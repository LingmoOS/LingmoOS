# For more information see the CMake documentation:
# https://cmake.org/documentation/
# https://community.kde.org/Guidelines_and_HOWTOs/CMake
##@@snippet_begin(adding_tw_1)
cmake_minimum_required(VERSION 3.0)
find_package(ECM 1.1.0 REQUIRED NO_MODULE)
##@@snippet_end

if(POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif()


if (NOT TARGET KF6ThreadWeaver)
  # Find the ThreadWeaver framework when we compiling this example stand-alone
##@@snippet_begin(adding_tw_2)
find_package(KF6ThreadWeaver ${KF_VERSION} REQUIRED)
##@@snippet_end
endif()

##@@snippet_begin(adding_tw_3)
# Define the project name
project(HelloWorld)
# Add the HelloWorld executable and link the ThreadWeaver
# library to it
add_executable(ThreadWeaver_HelloWorld HelloWorld.cpp)
target_link_libraries(ThreadWeaver_HelloWorld KF6::ThreadWeaver)
##@@snippet_end
