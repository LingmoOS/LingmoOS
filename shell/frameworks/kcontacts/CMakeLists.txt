cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "6.5.0") # handled by release scripts
set(KF_DEP_VERSION "6.4.0") # handled by release scripts

project(KContacts VERSION ${KF_VERSION})

# ECM setup
include(FeatureSummary)
find_package(ECM 6.4.0 NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
set(CMAKE_AUTORCC ON)

set(REQUIRED_QT_VERSION 6.5.0)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEGitCommitHooks)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)

include(ECMGenerateExportHeader)
include(ECMGenerateHeaders)
include(ECMGenerateExportHeader)
include(ECMDeprecationSettings)

include(ECMSetupVersion)
include(ECMQtDeclareLoggingCategory)
include(ECMAddQch)
include(ECMQmlModule)

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

set(KCONTACTS_LIB_VERSION ${KF_VERSION})

set(kcontacts_version_header "${CMAKE_CURRENT_BINARY_DIR}/src/kcontacts_version.h")
ecm_setup_version(PROJECT VARIABLE_PREFIX KCONTACTS
                        VERSION_HEADER "${kcontacts_version_header}"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6ContactsConfigVersion.cmake"
                        SOVERSION 6
)

########### Find packages ###########
find_package(Qt6 ${REQUIRED_QT_VERSION} NO_MODULE COMPONENTS Gui)
set_package_properties(Qt6Gui PROPERTIES
    TYPE REQUIRED
    PURPOSE "Basic library component"
)
find_package(KF6 ${KF_DEP_VERSION} REQUIRED COMPONENTS CoreAddons I18n Config Codecs)

########### Targets ###########

add_definitions(-DTRANSLATION_DOMAIN=\"kcontacts6\")
ki18n_install(po)

ecm_set_disabled_deprecation_versions(
    QT 6.5
    KF 5.240
)

add_subdirectory(src)

if(BUILD_TESTING)
    find_package(Qt6 ${REQUIRED_QT_VERSION} NO_MODULE COMPONENTS Quick)
    set_package_properties(Qt6Quick PROPERTIES
        TYPE OPTIONAL
        PURPOSE "QML integration test"
    )
    add_subdirectory(autotests)
    add_subdirectory(tests)
endif()

########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6Contacts")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF6Contacts_QCH
        FILE KF6ContactsQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF6ContactsQchTargets.cmake\")")
endif()

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF6ContactsConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6ContactsConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF6ContactsConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6ContactsConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF6ContactsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF6ContactsTargets.cmake NAMESPACE KF6::)

install(FILES
   ${kcontacts_version_header}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KContacts COMPONENT Devel
)

include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
