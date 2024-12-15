# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html.
# For more examples on how to use CMake, see https://github.com/android/ndk-samples.

# Sets the minimum CMake version required for this project.
cmake_minimum_required(VERSION 3.4.1)

# Declares the project name. The project name can be accessed via ${ PROJECT_NAME},
# Since this is the top level CMakeLists.txt, the project name is also accessible
# with ${CMAKE_PROJECT_NAME} (both CMake variables are in-sync within the top level
# build script scope).
project("cooperation")

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared Libs" FORCE)

# Requred to enable SIMD support
if (CMAKE_ANDROID_ARCH_ABI STREQUAL "arm64-v8a")
    set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} --target=aarch64-linux-android${ANDROID_VERSION}")
elseif (CMAKE_ANDROID_ARCH_ABI MATCHES "^armeabi.*")  # armeabi-v7a || armeabi-v6 || armeabi
    set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} --target=arm-linux-androideabi${ANDROID_VERSION}")
endif ()

set(PROJECT_ROOT_DIR ${CMAKE_SOURCE_DIR}/../../../../../..)
set(PROJECT_SOURCE_DIR ${PROJECT_ROOT_DIR})

# CMake module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_ROOT_DIR}/cmake)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_ROOT_DIR}/3rdparty/CppServer/cmake")

##===openssl====
##OpenSSL_1_1_1l openssl-3.0.1
#set(OPENSSL_VERSION "1.1.1l")
#set(HOST_TAG "linux-x86_64")
#set(OPENSSL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/openssl)
#
## OpenSSL - needs few executable such as perl and mv in PATH
##ExternalProject_Add(openssl_project
###        if [ ! -d openssl-${OPENSSL_VERSION} ]
###        then
###            if [ ! -f openssl-${OPENSSL_VERSION}.tar.gz ]
###        then
###            wget https://www.openssl.org/source/openssl-${OPENSSL_VERSION}.tar.gz || exit 128
###        fi
###            tar xzf openssl-${OPENSSL_VERSION}.tar.gz || exit 128
###        fi
##        URL https://www.openssl.org/source/openssl-${OPENSSL_VERSION}.tar.gz
###        execute_process(COMMAND bash tar xzf openssl-${OPENSSL_VERSION}.tar.gz WORK_PATH)
##        SOURCE_DIR openssl-${OPENSSL_VERSION}
###        GIT_REPOSITORY git@github.com:openssl/openssl.git
###        GIT_TAG ${OPENSSL_VERSION}
###        PATCH_COMMAND ${GIT} am ${CMAKE_CURRENT_SOURCE_DIR}/patches/0001-android-x86-target-Add-latomic-for-clang.patch
###        PATCH_COMMAND patch < ${CMAKE_CURRENT_SOURCE_DIR}/patches/0002-fix_android_so_version.patch
##        CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env PATH=${CMAKE_C_COMPILER_EXTERNAL_TOOLCHAIN}/bin:${ANDROID_NDK}/toolchains/llvm/prebuilt/${HOST_TAG}/bin:$ENV{PATH} SHLIB_EXT=.so CC=${CMAKE_C_COMPILER} ANDROID_NDK_HOME=${ANDROID_NDK} perl <SOURCE_DIR>/Configure android-${ANDROID_ARCH_NAME} -D__ANDROID_API__=${ANDROID_NATIVE_API_LEVEL} --openssldir=${CMAKE_LIBRARY_OUTPUT_DIRECTORY} --libdir="" --prefix=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
###        BUILD_COMMAND ${CMAKE_COMMAND} -E env PATH=${CMAKE_C_COMPILER_EXTERNAL_TOOLCHAIN}/bin:$ENV{PATH} ANDROID_NDK_ROOT=${ANDROID_NDK} SHLIB_EXT=.so make
##        BUILD_COMMAND ${CMAKE_COMMAND} -E env PATH=${ANDROID_NDK}/toolchains/llvm/prebuilt/${HOST_TAG}/bin:$ENV{PATH} ANDROID_NDK_HOME=${ANDROID_NDK} make SHLIB_EXT=.so
###        BUILD_BYPRODUCTS SHLIB_EXT=.so ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libssl.so ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libcrypto.so
###        BUILD_IN_SOURCE SHLIB_EXT=.so 1
##        )
#
#add_library(ssl SHARED IMPORTED)
##add_dependencies(ssl openssl_project)
##set_target_properties(ssl PROPERTIES VERSION 1.1)
##set_target_properties(ssl PROPERTIES IMPORTED_LOCATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libssl.so)
#set_target_properties(ssl PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/${ANDROID_ABI}/libssl.so)
#set(OPENSSL_SSL_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/lib/${ANDROID_ABI}/libssl.so)
#
#add_library(crypto SHARED IMPORTED)
##add_dependencies(crypto openssl_project)
##set_target_properties(crypto PROPERTIES VERSION 1.1)
##set_target_properties(crypto PROPERTIES IMPORTED_LOCATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libcrypto.so)
#set_target_properties(crypto PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/${ANDROID_ABI}/libcrypto.so)
#set(OPENSSL_CRYPTO_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/lib/${ANDROID_ABI}/libcrypto.so)
#
##=======

# header-only libraries: jwt, picojson
include_directories(${PROJECT_ROOT_DIR}/3rdparty/include)

# header: openssl
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#message("   >>> ANDROID_HOME  $ENV{ANDROID_HOME}")
#
#if (DEFINED ENV{ANDROID_HOME})
#    set(ssl_root_path $ENV{ANDROID_HOME}/android_openssl)
#    list(APPEND android_extra_libs
#            ${ssl_root_path}/ssl_1.1/${CMAKE_ANDROID_ARCH_ABI}/libcrypto_1_1.so
#            ${ssl_root_path}/ssl_1.1/${CMAKE_ANDROID_ARCH_ABI}/libssl_1_1.so)
#
#    set(OPENSSL_ROOT_DIR ${ssl_root_path}/ssl_1.1/${CMAKE_ANDROID_ARCH_ABI})
#    set(OPENSSL_INCLUDE_DIR ${ssl_root_path}/ssl_1.1/include)
#    set(OPENSSL_LIBRARIES ${android_extra_libs})
#    set(OPENSSL_VERSION 1.1)
#else()
#    message(FATAL_ERROR "ANDROID_HOME环境变量未设置！")
#endif()

# Modules cppserver
include("${PROJECT_ROOT_DIR}/modules/cppserver.cmake")


include_directories("${PROJECT_ROOT_DIR}/3rdparty/CppServer/include")
include_directories("${PROJECT_ROOT_DIR}/3rdparty/CppServer/modules")
include_directories("${PROJECT_ROOT_DIR}/3rdparty/CppServer/modules/CppLogging/include")
include_directories("${PROJECT_ROOT_DIR}/3rdparty/CppServer/modules/CppCommon/include")
include_directories("${PROJECT_ROOT_DIR}/3rdparty/CppServer/modules/asio/asio/include")
include_directories("${PROJECT_ROOT_DIR}/3rdparty/CppServer/modules/CppCommon/modules/fmt/include")

include_directories("${PROJECT_ROOT_DIR}/src")
include_directories("${PROJECT_ROOT_DIR}/src/lib/common")

#include_directories("${CMAKE_CURRENT_SOURCE_DIR}/proto")

# build proto
message("------proto----")
set(libproto_src_DIR ${PROJECT_ROOT_DIR}/src/lib/common/proto)
set(libproto_build_DIR ${CMAKE_BINARY_DIR}/proto)
# adds a source subdir with specifying a build output dir
add_subdirectory(${libproto_src_DIR} ${libproto_build_DIR})
# set include directories so dependent code can find the headers
include_directories(
        ${libproto_src_DIR}
        ${libproto_build_DIR}
)

# build session
message("------session-----")
set(libsession_src_DIR ${PROJECT_ROOT_DIR}/src/lib/common/session)
set(libsession_build_DIR ${CMAKE_BINARY_DIR}/session)
# adds a source subdir with specifying a build output dir
add_subdirectory(${libsession_src_DIR} ${libsession_build_DIR})
# set include directories so dependent code can find the headers
include_directories(
        ${libsession_src_DIR}
        ${libsession_build_DIR}
)

# build httpweb
message("------httpweb-----")
set(libhttpweb_src_DIR ${PROJECT_ROOT_DIR}/src/lib/common/httpweb)
set(libhttpweb_build_DIR ${CMAKE_BINARY_DIR}/httpweb)
# adds a source subdir with specifying a build output dir
add_subdirectory(${libhttpweb_src_DIR} ${libhttpweb_build_DIR})
include_directories(
        ${libhttpweb_src_DIR}
        ${libhttpweb_build_DIR}
)

FILE(GLOB MANAGER_SRC_FILES
        "${PROJECT_ROOT_DIR}/src/common/constant.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/manager/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/manager/*.cpp"
)
FILE(GLOB WRAP_SRC_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
#
# In this top level CMakeLists.txt, ${CMAKE_PROJECT_NAME} is used to define
# the target library name; in the sub-module's CMakeLists.txt, ${PROJECT_NAME}
# is preferred for the same purpose.
#
# In order to load a library into your app from Java/Kotlin, you must call
# System.loadLibrary() and pass the name of the library defined here;
# for GameActivity/NativeActivity derived applications, the same library name must be
# used in the AndroidManifest.xml file.
add_library(${CMAKE_PROJECT_NAME} SHARED
        # List C/C++ source files with relative paths to this CMakeLists.txt.
        ${MANAGER_SRC_FILES}
        ${WRAP_SRC_FILES}
        )

# Specifies libraries CMake should link to your target library. You
# can link libraries from various origins, such as libraries defined in this
# build script, prebuilt third-party libraries, or Android system libraries.
target_link_libraries(${CMAKE_PROJECT_NAME}
        # List libraries link to the target library
        android
        log
        httpweb
        session)