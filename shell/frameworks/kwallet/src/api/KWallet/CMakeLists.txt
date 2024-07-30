
include(ECMGenerateHeaders)

ecm_setup_version(${KF_VERSION} VARIABLE_PREFIX KWALLET
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kwallet_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6WalletConfigVersion.cmake"
                        SOVERSION 6)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kwallet_version.h
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KWallet COMPONENT Devel
)

if(APPLE)
    option(MAC_USE_OSXKEYCHAIN "On OS X, use the keychain as backend for kwallet, instead of kwalletd.")
else()
    set(MAC_USE_OSXKEYCHAIN FALSE)
endif()

add_library(KF6Wallet)
add_library(KF6::Wallet ALIAS KF6Wallet)

set_target_properties(KF6Wallet PROPERTIES
    VERSION     ${KWALLET_VERSION}
    SOVERSION   ${KWALLET_SOVERSION}
    EXPORT_NAME Wallet
)

if (MAC_USE_OSXKEYCHAIN)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    find_library(SECURITY_LIBRARY Security)
    target_sources(KF6Wallet PRIVATE
        kwallet_mac.cpp
    )
else()
    set(kwallet_dbus_SRCS)
    if (NOT EXCLUDE_DEPRECATED_BEFORE_AND_AT STREQUAL "CURRENT" AND
        EXCLUDE_DEPRECATED_BEFORE_AND_AT VERSION_LESS 5.72.0)
        set(kwallet_xml org.kde.KWallet.xml)
    else()
        # copy of org.kde.KWallet.xml, but with all deprecated API removed
        set(kwallet_xml org.kde.KWallet.nodeprecated.xml)
    endif()
    qt_add_dbus_interface(kwallet_dbus_SRCS ${kwallet_xml} kwallet_interface)
    target_sources(KF6Wallet PRIVATE
        kwallet.cpp
        ${kwallet_dbus_SRCS}
    )
endif()

ecm_qt_declare_logging_category(KF6Wallet
    HEADER kwallet_api_debug.h
    IDENTIFIER KWALLET_API_LOG
    CATEGORY_NAME kf.wallet.api
    OLD_CATEGORY_NAMES kf5.kwallet.api
    DESCRIPTION "kwallet api"
    EXPORT KWALLET
)

ecm_generate_export_header(KF6Wallet
    BASE_NAME KWallet
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 5.72
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_include_directories(KF6Wallet INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KWallet>")

target_link_libraries(KF6Wallet
    PUBLIC
        Qt6::Gui
    PRIVATE
        Qt6::DBus
        Qt6::Widgets
        KF6::ConfigCore   # used to store the wallet to be used
)

if(MAC_USE_OSXKEYCHAIN)
   target_link_libraries(KF6Wallet PRIVATE ${CARBON_LIBRARY}
                                           ${SECURITY_LIBRARY}
                                           ${COREFOUNDATION_LIBRARY})
elseif(APPLE)
    target_link_libraries(KF6Wallet PRIVATE ${CARBON_LIBRARY})
else()
    target_link_libraries(KF6Wallet PRIVATE Qt6::DBus)
    install(FILES ${kwallet_xml} DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR} RENAME kf6_org.kde.KWallet.xml)
endif()

ecm_generate_headers(KWallet_HEADERS
  HEADER_NAMES
  KWallet

  REQUIRED_HEADERS KWallet_HEADERS
)

install(TARGETS KF6Wallet EXPORT KF6WalletTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kwallet_export.h
  ${KWallet_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KWallet COMPONENT Devel
)

if(BUILD_QCH)
    ecm_add_qch(
        KF6Wallet_QCH
        NAME KWallet
        BASE_NAME KF6Wallet
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KWallet_HEADERS}
        LINK_QCHS
            Qt6Gui_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            KWALLET_EXPORT
            KWALLET_DEPRECATED
            KWALLET_DEPRECATED_EXPORT
            "KWALLET_DEPRECATED_VERSION(x, y, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()


# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6Wallet")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF6Wallet_QCH
        FILE KF6WalletQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF6WalletQchTargets.cmake\")")
endif()

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF6WalletConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6WalletConfig.cmake"
  PATH_VARS KDE_INSTALL_DBUSINTERFACEDIR
  PATH_VARS KDE_INSTALL_BINDIR
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF6WalletConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6WalletConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF6WalletTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF6WalletTargets.cmake NAMESPACE KF6:: )

