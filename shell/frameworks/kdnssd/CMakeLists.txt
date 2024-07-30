cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "6.5.0") # handled by release scripts
project(KDNSSD VERSION ${KF_VERSION})

include(FeatureSummary)
find_package(ECM 6.4.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/modules) #TODO: remove cmake/modules when extra-cmake-modules is done

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)

include(KDEGitCommitHooks)
include(ECMPoQmTools)
include(ECMAddQch)
include(ECMGenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMDeprecationSettings)
include(CMakePackageConfigHelpers)

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

set(REQUIRED_QT_VERSION 6.5.0)
find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Network)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(kdnssd_version_header "${CMAKE_CURRENT_BINARY_DIR}/src/kdnssd_version.h")
ecm_setup_version(PROJECT VARIABLE_PREFIX KDNSSD
    VERSION_HEADER "${kdnssd_version_header}"
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6DNSSDConfigVersion.cmake"
    SOVERSION 6)
install(FILES
    ${kdnssd_version_header}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KDNSSD
    COMPONENT Devel
)

configure_file(config-kdnssd.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kdnssd.h )


find_package(Avahi)
set_package_properties(Avahi PROPERTIES DESCRIPTION "Facilities for service discovery on a local network (DNSSD)"
                       URL "https://avahi.org"
                       TYPE OPTIONAL
                       PURPOSE "Either Avahi or DNSSD is required for KDE applications to make use of multicast DNS/DNS-SD service discovery"
                      )

if(NOT AVAHI_FOUND)
    find_package(DNSSD)
    set_package_properties(DNSSD PROPERTIES DESCRIPTION "Facilities for service discovery on a local network"
                          URL "https://avahi.org"
                          TYPE OPTIONAL
                          PURPOSE "Either Avahi or DNSSD is required for KDE applications to make use of multicast DNS/DNS-SD service discovery"
                          )
endif( )

if(AVAHI_FOUND OR DNSSD_FOUND)
    find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED DBus)
endif()

ecm_install_po_files_as_qm(poqm)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)
ecm_set_disabled_deprecation_versions(
    QT 6.5
)

add_subdirectory(src)

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6DNSSD")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF6DNSSD_QCH
        FILE KF6DNSSDQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF6DNSSDQchTargets.cmake\")")
endif()

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KF6DNSSDConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KF6DNSSDConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KF6DNSSDConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KF6DNSSDConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

install(EXPORT KF6DNSSDTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE KF6DNSSDTargets.cmake NAMESPACE KF6:: COMPONENT Devel)

include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)


kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
