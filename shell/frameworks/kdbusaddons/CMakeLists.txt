cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "6.5.0") # handled by release scripts
project(KDBusAddons VERSION ${KF_VERSION})

include(FeatureSummary)
find_package(ECM 6.4.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)


set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(KDEGitCommitHooks)
include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)

set(REQUIRED_QT_VERSION 6.5.0)
find_package(Qt6DBus ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)
find_package(Qt6Gui ${REQUIRED_QT_VERSION} CONFIG) # qtx11extras_p.h

include(GenerateExportHeader)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMGenerateExportHeader)
include(ECMMarkNonGuiExecutable)
include(ECMAddQch)

include(ECMPoQmTools)

set(kdbusaddons_version_header "${CMAKE_CURRENT_BINARY_DIR}/src/kdbusaddons_version.h")
ecm_setup_version(PROJECT VARIABLE_PREFIX KDBUSADDONS
                        VERSION_HEADER "${kdbusaddons_version_header}"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6DBusAddonsConfigVersion.cmake"
                        SOVERSION 6)

option(WITH_X11 "Build X11 support" ON)

if (WIN32 OR APPLE OR ANDROID)
    set(WITH_X11 OFF)
endif()

if (WITH_X11)
    set(HAVE_X11 TRUE)
else()
    set(HAVE_X11 FALSE)
endif()

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

ecm_set_disabled_deprecation_versions(
    QT 6.7
)

ecm_install_po_files_as_qm(poqm)

add_subdirectory(src)
if (BUILD_TESTING)
    add_subdirectory(autotests)
    add_subdirectory(tests)
endif()

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6DBusAddons")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF6DBusAddons_QCH
        FILE KF6DBusAddonsQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF6DBusAddonsQchTargets.cmake\")")
endif()

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KF6DBusAddonsConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KF6DBusAddonsConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KF6DBusAddonsConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KF6DBusAddonsConfigVersion.cmake"
               "KF6DBusAddonsMacros.cmake"
               "KF6dbus.service.in"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

install(EXPORT KF6DBusAddonsTargets  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"  FILE KF6DBusAddonsTargets.cmake  NAMESPACE KF6:: )

install(FILES ${kdbusaddons_version_header} DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KDBusAddons COMPONENT Devel)

include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
