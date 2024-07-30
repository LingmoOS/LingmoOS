cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "6.5.0") # handled by release scripts
set(KF_DEP_VERSION "6.4.0") # handled by release scripts
project (KPeople VERSION ${KF_VERSION})

include(FeatureSummary)
find_package(ECM 6.4.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)

include(KDEGitCommitHooks)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMSetupVersion)
include(ECMAddTests)
include(ECMAddQch)
include(CMakePackageConfigHelpers)
include(ECMGenerateExportHeader)
include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)
include(ECMQmlModule)

set(REQUIRED_QT_VERSION 6.5.0)
find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Gui Sql Widgets Qml Quick)

# shall we use DBus?
# enabled per default on Linux & BSD systems
set(USE_DBUS_DEFAULT OFF)
if(UNIX AND NOT APPLE AND NOT ANDROID AND NOT HAIKU)
    set(USE_DBUS_DEFAULT ON)
endif()
option(USE_DBUS "Build components using DBus" ${USE_DBUS_DEFAULT})
if(USE_DBUS)
    find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED DBus)
    set(HAVE_QTDBUS ${Qt6DBus_FOUND})
endif()

find_package(KF6CoreAddons ${KF_DEP_VERSION} CONFIG REQUIRED)
find_package(KF6WidgetsAddons ${KF_DEP_VERSION} CONFIG REQUIRED)
find_package(KF6I18n ${KF_DEP_VERSION} CONFIG REQUIRED)
find_package(KF6ItemViews ${KF_DEP_VERSION} CONFIG REQUIRED)
find_package(KF6Contacts ${KF_DEP_VERSION} CONFIG REQUIRED)

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

add_definitions(-DTRANSLATION_DOMAIN=\"kpeople6\")

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6People")

set(kpeople_version_header "${CMAKE_CURRENT_BINARY_DIR}/src/kpeople_version.h")
ecm_setup_version(PROJECT VARIABLE_PREFIX KPEOPLE
                        VERSION_HEADER "${kpeople_version_header}"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6PeopleConfigVersion.cmake"
                        SOVERSION 6)

option(ENABLE_EXAMPLES "Build the examples" "ON")

ki18n_install(po)

ecm_set_disabled_deprecation_versions(
    QT 6.5
    KF 5.102
)

add_subdirectory (src)
if (ENABLE_EXAMPLES)
  add_subdirectory(examples)
endif()
if (BUILD_TESTING)
    add_subdirectory(autotests)
endif()

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF6People_QCH
        FILE KF6PeopleQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF6PeopleQchTargets.cmake\")")
endif()

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF6PeopleConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6PeopleConfig.cmake"
  INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

install(EXPORT KPeopleTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KPeopleTargets.cmake NAMESPACE KF6::)
install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KF6PeopleConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KF6PeopleConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

install(
    FILES ${kpeople_version_header}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KPeople
    COMPONENT Devel
)

include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
