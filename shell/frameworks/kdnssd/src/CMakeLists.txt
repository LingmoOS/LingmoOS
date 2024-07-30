add_library(KF6DNSSD)
set_target_properties(KF6DNSSD PROPERTIES
   VERSION     "${KDNSSD_VERSION}"
   SOVERSION   "${KDNSSD_SOVERSION}"
   EXPORT_NAME DNSSD
)

ecm_create_qm_loader(KF6DNSSD kdnssd6_qt)

target_sources(KF6DNSSD PRIVATE
    servicebase.cpp
    servicemodel.cpp
    domainmodel.cpp
)

if (AVAHI_FOUND)
    include_directories( ${AVAHI_INCLUDE_DIR} )
    target_sources(KF6DNSSD PRIVATE
        avahi-domainbrowser.cpp
        avahi-servicebrowser.cpp
        avahi-remoteservice.cpp
        avahi-publicservice.cpp
        avahi-servicetypebrowser.cpp
        avahi_server_interface.cpp
        avahi_serviceresolver_interface.cpp
        avahi_entrygroup_interface.cpp
        avahi_listener.cpp
    )
    set(kdnssd_dbus_LIB_SRCS)
    qt_add_dbus_interface(kdnssd_dbus_LIB_SRCS org.freedesktop.Avahi.DomainBrowser.xml avahi_domainbrowser_interface)
    qt_add_dbus_interface(kdnssd_dbus_LIB_SRCS org.freedesktop.Avahi.ServiceBrowser.xml avahi_servicebrowser_interface)
    qt_add_dbus_interface(kdnssd_dbus_LIB_SRCS org.freedesktop.Avahi.ServiceTypeBrowser.xml avahi_servicetypebrowser_interface)
    target_sources(KF6DNSSD PRIVATE ${kdnssd_dbus_LIB_SRCS})
elseif (DNSSD_FOUND)
    include_directories( ${DNSSD_INCLUDE_DIR} )
    target_sources(KF6DNSSD PRIVATE
        mdnsd-domainbrowser.cpp
        mdnsd-remoteservice.cpp
        mdnsd-publicservice.cpp
        mdnsd-responder.cpp
        mdnsd-servicebrowser.cpp
        mdnsd-servicetypebrowser.cpp
    )
else ()
    target_sources(KF6DNSSD PRIVATE
        dummy-domainbrowser.cpp
        dummy-remoteservice.cpp
        dummy-publicservice.cpp
        dummy-servicebrowser.cpp
        dummy-servicetypebrowser.cpp
    )

endif ()

ecm_generate_export_header(KF6DNSSD
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/kdnssd_export.h
    BASE_NAME KDNSSD
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

# Apps must include <KDNSSD/File> or <kdnssd/file.h>
target_include_directories(KF6DNSSD INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KDNSSD>")

target_link_libraries(KF6DNSSD PUBLIC Qt6::Network)

if (AVAHI_FOUND OR DNSSD_FOUND)
    target_link_libraries(KF6DNSSD PRIVATE Qt6::DBus)
endif ()

if (DNSSD_FOUND)
  target_link_libraries(KF6DNSSD PRIVATE ${DNSSD_LIBRARIES})
endif ()

set(KDNSSD_BUILD_INCLUDE_DIRS ${KDNSSD_BINARY_DIR})
target_include_directories(KF6DNSSD PUBLIC "$<BUILD_INTERFACE:${KDNSSD_BUILD_INCLUDE_DIRS}>")

ecm_generate_headers(KDNSSD_CamelCase_HEADERS
  HEADER_NAMES
  DomainBrowser
  RemoteService
  ServiceTypeBrowser
  PublicService
  ServiceBase
  ServiceBrowser
  ServiceModel
  DomainModel

  PREFIX KDNSSD
  REQUIRED_HEADERS kdnssd_HEADERS
)

install(FILES ${KDNSSD_CamelCase_HEADERS} DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KDNSSD/KDNSSD COMPONENT Devel)

install(TARGETS KF6DNSSD EXPORT KF6DNSSDTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kdnssd_export.h
  ${kdnssd_HEADERS}
  DESTINATION  ${KDE_INSTALL_INCLUDEDIR_KF}/KDNSSD/kdnssd COMPONENT Devel
)

if(BUILD_QCH)
    ecm_add_qch(
        KF6DNSSD_QCH
        NAME KDNSSD
        BASE_NAME KF6DNSSD
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${kdnssd_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt6Network_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
            ${KDNSSD_BUILD_INCLUDE_DIRS}
        BLANK_MACROS
            KDNSSD_EXPORT
            KDNSSD_DEPRECATED
            KDNSSD_DEPRECATED_EXPORT
            "KDNSSD_DEPRECATED_VERSION(x, y, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

