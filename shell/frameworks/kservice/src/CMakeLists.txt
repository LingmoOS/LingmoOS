include(CheckSymbolExists)
include(CheckFunctionExists)
check_function_exists(mmap HAVE_MMAP)
check_symbol_exists(posix_madvise "sys/mman.h" HAVE_MADVISE)
configure_file(config-ksycoca.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ksycoca.h )

add_library(KF6Service)
add_library(KF6::Service ALIAS KF6Service)

set_target_properties(KF6Service PROPERTIES
    VERSION     ${KSERVICE_VERSION}
    SOVERSION   ${KSERVICE_SOVERSION}
    EXPORT_NAME Service
)

target_sources(KF6Service PRIVATE
   services/kapplicationtrader.cpp
   services/kmimetypefactory.cpp
   services/kservice.cpp
   services/kserviceaction.cpp
   services/kservicefactory.cpp
   services/kservicegroup.cpp
   services/kservicegroupfactory.cpp
   services/kserviceoffer.cpp
   sycoca/ksycoca.cpp
   sycoca/ksycocadevices.cpp
   sycoca/ksycocadict.cpp
   sycoca/ksycocaentry.cpp
   sycoca/ksycocafactory.cpp
   sycoca/kmemfile.cpp
   sycoca/kbuildmimetypefactory.cpp
   sycoca/kbuildservicefactory.cpp
   sycoca/kbuildservicegroupfactory.cpp
   sycoca/kbuildsycoca.cpp
   sycoca/kctimefactory.cpp
   sycoca/kmimeassociations.cpp
   sycoca/vfolder_menu.cpp
)

ecm_qt_declare_logging_category(KF6Service
    HEADER servicesdebug.h
    IDENTIFIER SERVICES
    CATEGORY_NAME kf.service.services
    OLD_CATEGORY_NAMES kf5.kservice.services
    EXPORT KSERVICE
    DESCRIPTION "service (kservice lib)"
)
ecm_qt_declare_logging_category(KF6Service
    HEADER sycocadebug.h
    IDENTIFIER SYCOCA
    CATEGORY_NAME kf.service.sycoca
    OLD_CATEGORY_NAMES kf5.kservice.sycoca
    EXPORT KSERVICE
    DESCRIPTION "sycoca (kservice)"
)

ecm_generate_export_header(KF6Service
    BASE_NAME KService
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 5.90
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

set(kservice_includes
   ${CMAKE_CURRENT_BINARY_DIR}/.. # Since we publicly include kservice_version.h
   ${CMAKE_CURRENT_SOURCE_DIR}/services
   ${CMAKE_CURRENT_SOURCE_DIR}/sycoca
)
target_include_directories(KF6Service
    PUBLIC
        "$<BUILD_INTERFACE:${kservice_includes}>"
    INTERFACE
        "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KService>"
)

target_link_libraries(KF6Service
PUBLIC
    Qt6::Core
PRIVATE
    Qt6::Xml  # (for vfolder menu) QDomDocument
    KF6::ConfigCore   # KConfig and friends
    KF6::CoreAddons   # KShell
)

ecm_generate_headers(KService_HEADERS
  HEADER_NAMES
  KSycoca
  KSycocaEntry
  KSycocaType

  RELATIVE sycoca REQUIRED_HEADERS KService_HEADERS
)
ecm_generate_headers(KService_HEADERS
  HEADER_NAMES
  KApplicationTrader
  KService
  KServiceAction
  KServiceGroup

  RELATIVE services REQUIRED_HEADERS KService_HEADERS
)

install(TARGETS KF6Service EXPORT KF6ServiceTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
   "${CMAKE_CURRENT_BINARY_DIR}/kservice_export.h"
   ${KService_HEADERS}
   ${CMAKE_CURRENT_SOURCE_DIR}/services/kserviceconversioncheck_p.h # helper header included by kservice.h
   DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KService COMPONENT Devel
)

ecm_qt_install_logging_categories(
    EXPORT KSERVICE
    FILE kservice.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

if(BUILD_QCH)
    ecm_add_qch(
        KF6Service_QCH
        NAME KService
        BASE_NAME KF6Service
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KService_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            KF6Config_QCH
            KF6CoreAddons_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
            ${kservice_includes}
        BLANK_MACROS
            KSERVICE_EXPORT
            KSERVICE_DEPRECATED
            KSERVICE_DEPRECATED_EXPORT
            "KSERVICE_DEPRECATED_VERSION(x, y, t)"
            "KSERVICE_DEPRECATED_VERSION_BELATED(x, y, xt, yt, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

add_subdirectory(kbuildsycoca)

