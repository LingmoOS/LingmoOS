configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-package.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-package.h)

add_library(KF6Package)
add_library(KF6::Package ALIAS KF6Package)

set_target_properties(KF6Package PROPERTIES
    VERSION     ${PACKAGE_VERSION}
    SOVERSION   ${PACKAGE_SOVERSION}
    EXPORT_NAME Package
)

target_sources(KF6Package PRIVATE
    package.cpp
    packagestructure.cpp
    packageloader.cpp
    packagejob.cpp
    private/packages.cpp
    private/packagejobthread.cpp
)

ecm_qt_declare_logging_category(KF6Package
    HEADER kpackage_debug.h
    IDENTIFIER KPACKAGE_LOG
    CATEGORY_NAME kf.package
    OLD_CATEGORY_NAMES kf5.kpackage
    DESCRIPTION "kpackage (lib)"
    EXPORT KPACKAGE
)

ecm_generate_export_header(KF6Package
    EXPORT_FILE_NAME kpackage/package_export.h
    BASE_NAME KPackage
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_link_libraries(KF6Package
PUBLIC
     KF6::CoreAddons
PRIVATE
     KF6::Archive
     KF6::I18n
)

if (HAVE_DBUS)
    target_link_libraries(KF6Package
        PRIVATE
        Qt::DBus # notification
    )
    target_compile_definitions(KF6Package PRIVATE -DHAVE_QTDBUS=1)
else()
    target_compile_definitions(KF6Package PRIVATE -DHAVE_QTDBUS=0)
endif()

target_include_directories(KF6Package PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..;${CMAKE_CURRENT_BINARY_DIR};${CMAKE_CURRENT_BINARY_DIR}/KPackage>"
)
target_include_directories(KF6Package INTERFACE
  "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KPackage>"
)

########### install files ###############
ecm_generate_headers(Package_CamelCase_HEADERS
    HEADER_NAMES
        Package
        PackageStructure
        PackageLoader
        PackageJob
        packagestructure_compat_p
    REQUIRED_HEADERS Package_HEADERS
    PREFIX KPackage
)

install(FILES
        ${Package_HEADERS}
        ${CMAKE_CURRENT_BINARY_DIR}/kpackage/package_export.h
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KPackage/kpackage COMPONENT Devel)

install(FILES
        ${Package_CamelCase_HEADERS}
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KPackage/KPackage COMPONENT Devel)

install(TARGETS KF6Package EXPORT KF6PackageTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
if (NOT BUILD_SHARED_LIBS)
    install(TARGETS kpackage_common_STATIC EXPORT KF6PackageTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
endif()

if(BUILD_QCH)
    ecm_add_qch(
        KF6Package_QCH
        NAME KPackage
        BASE_NAME KF6Package
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${Package_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            KF6CoreAddons_QCH
        BLANK_MACROS
            KPACKAGE_EXPORT
            "KPACKAGE_DEPRECATED_VERSION(x, y, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()
