cmake_minimum_required(VERSION 3.4)
project(QtZeroConf VERSION 0.1.0)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(PUBLIC_HEADERS
    qzeroconfservice.h
    qzeroconfglobal.h
    qzeroconf.h
)
add_library(QtZeroConf
    ${PUBLIC_HEADERS}
    qzeroconfservice.cpp
)

include(GNUInstallDirs)

if(BUILD_SHARED_LIBS)
    target_compile_definitions(QtZeroConf PRIVATE QT_BUILD_ZEROCONF_LIB)
    set_target_properties(QtZeroConf PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 0)
else()
    target_compile_definitions(QtZeroConf PUBLIC QZEROCONF_STATIC)
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux" OR ${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")
    find_library(avahi-client-lib avahi-client REQUIRED)
    find_library(avahi-common-lib avahi-common REQUIRED)
    find_path(avahi-client-includes avahi-client/client.h REQUIRED)
    find_path(avahi-common-includes avahi-common/defs.h REQUIRED)
    target_sources(QtZeroConf PRIVATE
        avahi-qt/qt-watch_p.h
        avahi-qt/qt-watch.cpp
        avahiclient.cpp
    )
    target_include_directories(QtZeroConf PRIVATE ${avahi-client-includes} ${avahi-common-includes})
    list(APPEND ${PUBLIC_HEADERS}
        avahi-qt/qt-watch.h
    )
    target_link_libraries(QtZeroConf PRIVATE ${avahi-client-lib} ${avahi-common-lib})
endif()

if(APPLE)
    target_sources(QtZeroConf PRIVATE
        bonjour_p.h
        bonjour.cpp
    )
    find_library(CoreServices CoreServices)
    target_link_libraries(QtZeroConf PUBLIC ${CoreServices})
endif()

target_include_directories(QtZeroConf PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/QtZeroConf>
)
target_link_libraries(QtZeroConf PUBLIC Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Network)

if(WIN32)
    target_sources(QtZeroConf PRIVATE
        bonjour_p.h
        bonjour.cpp
        bonjour-sdk/dnssd_clientlib.c
        bonjour-sdk/dnssd_clientstub.c
        bonjour-sdk/dnssd_ipc.c
    )
    target_link_libraries(QtZeroConf PUBLIC ws2_32)
    if(MSVC)
        target_link_libraries(QtZeroConf PUBLIC "legacy_stdio_definitions.lib")
    endif()
    target_compile_definitions(QtZeroConf PRIVATE
        -DWIN32
        -D_USRDLL
        -DMDNS_DEBUGMSGS=0
        -DWIN32_LEAN_AND_MEAN
        -DUSE_TCP_LOOPBACK
        -D_NO_CRT_STDIO_INLINE
        -D_CRT_SECURE_NO_DEPRECATE
        -D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1
        -DNOT_HAVE_SA_LEN)
    target_include_directories(QtZeroConf PRIVATE "${CMAKE_CURRENT_LIST_DIR}/bonjour-sdk")
endif()

if(ANDROID_AVAHI)
     set(ACM "${CMAKE_CURRENT_LIST_DIR}/avahi-common")
     set(ACR "${CMAKE_CURRENT_LIST_DIR}/avahi-core")
     target_sources(QtZeroConf PRIVATE
        qzeroconf.h
        avahi-qt/qt-watch.h
        avahi-qt/qt-watch_p.h
        avahicore.cpp
        avahi-qt/qt-watch.cpp
        ${ACM}/address.c
        ${ACM}/alternative.c
        ${ACM}/domain.c
        ${ACM}/error.c
        ${ACM}/i18n.c
        ${ACM}/malloc.c
        ${ACM}/rlist.c
        ${ACM}/simple-watch.c
        ${ACM}/strlst.c
        ${ACM}/thread-watch.c
        ${ACM}/timeval.c
        ${ACM}/utf8.c
        ${ACR}/addr-util.c
        ${ACR}/announce.c
        ${ACR}/browse.c
        ${ACR}/browse-dns-server.c
        ${ACR}/browse-domain.c
        ${ACR}/browse-service.c
        ${ACR}/browse-service-type.c
        ${ACR}/cache.c
        ${ACR}/dns.c
        ${ACR}/domain-util.c
        ${ACR}/entry.c
        ${ACR}/fdutil.c
        ${ACR}/hashmap.c
        ${ACR}/iface.c
        ${ACR}/iface-linux.c
        ${ACR}/log.c
        ${ACR}/multicast-lookup.c
        ${ACR}/netlink.c
        ${ACR}/prioq.c
        ${ACR}/probe-sched.c
        ${ACR}/querier.c
        ${ACR}/query-sched.c
        ${ACR}/resolve-address.c
        ${ACR}/resolve-host-name.c
        ${ACR}/resolve-service.c
        ${ACR}/response-sched.c
        ${ACR}/rr.c
        ${ACR}/rrlist.c
        ${ACR}/server.c
        ${ACR}/socket.c
        ${ACR}/timeeventq.c
        ${ACR}/util.c
        ${ACR}/wide-area.c
    )
    list(APPEND ${PUBLIC_HEADERS}
        avahi-qt/qt-watch.h
    )
    target_compile_definitions(QtZeroConf PRIVATE HAVE_STRLCPY GETTEXT_PACKAGE HAVE_NETLINK)
elseif(ANDROID)
    target_sources(QtZeroConf PRIVATE
        qzeroconf.h
        androidnsd_p.h
        androidnsd.cpp
    )
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui)
    target_link_libraries(QtZeroConf PUBLIC Qt${QT_VERSION_MAJOR}::Gui)
    if (QT_VERSION_MAJOR EQUAL 5)
        find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS AndroidExtras)
        target_link_libraries(QtZeroConf PUBLIC Qt${QT_VERSION_MAJOR}::AndroidExtras)
    endif ()
endif()

# install
set(INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}" CACHE STRING "Installation directory for cmake config files")
set_target_properties(QtZeroConf PROPERTIES PUBLIC_HEADER
    "${PUBLIC_HEADERS}"
)
install(TARGETS QtZeroConf
    EXPORT QtZeroConfConfig
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
)
export(TARGETS QtZeroConf
    FILE ${CMAKE_CURRENT_BINARY_DIR}/QtZeroConfConfig.cmake
)
install(EXPORT QtZeroConfConfig
    DESTINATION ${INSTALL_CMAKEDIR}
)

set(BUILD_EXAMPLE false)
if(BUILD_EXAMPLE)
    add_subdirectory(example)
endif()
