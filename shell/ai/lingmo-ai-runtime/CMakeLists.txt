cmake_minimum_required(VERSION 3.5)

project(lingmo-ai-runtime LANGUAGES CXX C)

set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_SKIP_RPATH ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
find_package(LingmoAiEngine REQUIRED)
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

find_package(DBus1 REQUIRED)
include_directories(${DBus1_INCLUDE_DIRS})

find_library(GLIBC_LIB glib-2.0)

pkg_check_modules(GIO REQUIRED gio-unix-2.0)
pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0)

include_directories(${GIO_INCLUDE_DIRS})
include(GNUInstallDirs)
include(cmake/engine-plugin-path-defs.cmake)
include_directories(
    ./src
)

set(CPR_THREADPOOL_SOURCES
    src/services/threadpool/singleton.h
    src/services/threadpool/async.h
    src/services/threadpool/async.cpp
    src/services/threadpool/threadpool.h
    src/services/threadpool/threadpool.cpp
)

add_executable(lingmo-ai-runtime
    src/rsa2048.h src/rsa2048.cpp
    src/encryption.h src/encryption.cpp
    src/pbkdf2.h src/pbkdf2.cpp
    src/aes256.h src/aes256.cpp
    src/engine/aienginepluginmanager.h src/engine/aienginepluginmanager.cpp
    src/engine/library.h src/engine/library.cpp
    src/services/server.cpp
    src/services/server.h
    src/services/serverproxy.cpp
    src/services/serverproxy.h
    src/services/servicemanager.cpp
    src/services/servicemanager.h
    src/services/textprocessor.cpp
    src/services/textprocessor.h
    src/services/textprocessorglue.c
    src/services/textprocessorglue.h
    src/services/speechprocessor.cpp
    src/services/speechprocessor.h
    src/services/speechprocessorglue.c
    src/services/speechprocessorglue.h
    src/services/visionprocessor.cpp
    src/services/visionprocessor.h
    src/services/visionprocessorglue.c
    src/services/visionprocessorglue.h
#    src/services/keywordrecognizer.cpp
#    src/services/keywordrecognizer.h
#    src/services/keywordrecognizerglue.h
#    src/services/keywordrecognizerglue.c
    src/engine/aienginepluginmanager.h src/engine/aienginepluginmanager.cpp
    src/engine/library.h src/engine/library.cpp
    src/config/engineconfiguration.h src/config/engineconfiguration.cpp
    src/engine/aienginemanager.h src/engine/aienginemanager.cpp
    src/config/prompttemplate.h src/config/prompttemplate.cpp
    main.cpp
    src/microphone/microphone.h src/microphone/microphone.cpp
    ${CPR_THREADPOOL_SOURCES}
    src/defaultkeymanager/defaultkeymanager.h src/defaultkeymanager/defaultkeymanager.cpp
)

target_link_libraries(
    lingmo-ai-runtime
    pthread
    jsoncpp
    crypto
    dl
    ${GIO_LIBRARIES}
    ${PORTAUDIO_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${DBus1_LIBRARIES}
    ${GLIBC_LIB}
)

if (ENABLE_TEST)
    enable_testing()
    add_subdirectory(test)
endif ()

include(CMakePackageConfigHelpers)

install(TARGETS lingmo-ai-runtime DESTINATION /usr/sbin)
install(DIRECTORY configs/etc/ DESTINATION /etc)
install(DIRECTORY configs/lib/ DESTINATION /lib)
