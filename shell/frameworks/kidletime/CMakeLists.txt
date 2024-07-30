cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "6.5.0") # handled by release scripts
project(KIdleTime VERSION ${KF_VERSION})

# ECM setup
include(FeatureSummary)
find_package(ECM 6.4.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)

include(KDEGitCommitHooks)
include(CMakeFindFrameworks)
include(CMakePackageConfigHelpers)
include(ECMGenerateExportHeader)
include(ECMQtDeclareLoggingCategory)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMDeprecationSettings)
include(ECMAddQch)

set(kiconthemes_version_header "${CMAKE_CURRENT_BINARY_DIR}/src/kidletime_version.h")
ecm_setup_version(PROJECT VARIABLE_PREFIX KIDLETIME
                        VERSION_HEADER "${kiconthemes_version_header}"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6IdleTimeConfigVersion.cmake"
                        SOVERSION 6)

set(REQUIRED_QT_VERSION 6.5.0)
find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Gui)

option(WITH_X11 "Build X11 support" ON)
option(WITH_WAYLAND "BUILD Wayland support" ON)

if(APPLE OR WIN32)
    set(WITH_X11 OFF)
    set(WITH_WAYLAND OFF)
endif()

if(APPLE)
    cmake_find_frameworks(CoreFoundation Carbon)
endif()

if(WITH_X11)
    find_package(X11 REQUIRED)
    find_package(XCB REQUIRED COMPONENTS XCB SYNC)
    find_package(X11_XCB REQUIRED)
    set(HAVE_X11 TRUE)

    if (X11_XSync_FOUND AND XCB_XCB_FOUND AND X11_XCB_FOUND AND XCB_SYNC_FOUND)
        # One cannot just use "set(HAVE_XSYNC X11_XSync_FOUND AND XCB_XCB_FOUND...)"
        # Doing so would create a list because the value passed to set() is not evaluated.
        set(HAVE_XSYNC TRUE)
    else()
        set(HAVE_XSYNC FALSE)
    endif()
    add_feature_info("libXext" X11_XSync_FOUND "required for X-Sync based poller")
    add_feature_info("libX11-xcb" X11_XCB_FOUND "required for X-Sync based poller")
    add_feature_info("libxcb" XCB_XCB_FOUND "required for X-Sync based poller")
    add_feature_info("libxcb-sync" XCB_SYNC_FOUND "required for X-Sync based poller")
    add_feature_info("X-Sync based poller" HAVE_XSYNC "efficient operation of KIdleTime. STRONGLY RECOMMENDED (requires libXext, libX11-xcb, libxcb and libxcb-sync)")

    set(HAVE_XSCREENSAVER ${X11_Xscreensaver_FOUND})
    add_feature_info("X-Screensaver based poller" HAVE_XSCREENSAVER "KIdleTime fallback mode (libXss)")

    if(HAVE_XSCREENSAVER)
        find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED DBus)
    endif()

    if(NOT HAVE_XSCREENSAVER AND NOT HAVE_XSYNC)
        message(FATAL_ERROR "\nNeither the XSync (libXext) nor XScreensaver (libXss) development package was found.\nPlease install one of them (XSync is recommended)")
    endif()
else()
    set(HAVE_X11 FALSE)
endif()

if(WITH_WAYLAND)
    find_package(Qt6Gui REQUIRED CONFIG COMPONENTS Private)
    find_package(Qt6WaylandClient ${REQUIRED_QT_VERSION} REQUIRED CONFIG)
    find_package(Wayland 1.9 REQUIRED Client)
    find_package(PlasmaWaylandProtocols 1.6.0 CONFIG REQUIRED)
    find_package(WaylandProtocols 1.27)
    set_package_properties(WaylandProtocols PROPERTIES
        TYPE REQUIRED
        PURPOSE "Collection of Wayland protocols that add functionality not available in the Wayland core protocol"
        URL "https://gitlab.freedesktop.org/wayland/wayland-protocols/"
    )
endif()

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

ecm_set_disabled_deprecation_versions(
    QT 6.5
)

# Subdirectories
add_subdirectory(src)
add_subdirectory(examples)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6IdleTime")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF6IdleTime_QCH
        FILE KF6IdleTimeQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF6IdleTimeQchTargets.cmake\")")
endif()

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KF6IdleTimeConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KF6IdleTimeConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KF6IdleTimeConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KF6IdleTimeConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

install(EXPORT KF6IdleTimeTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF6IdleTimeTargets.cmake NAMESPACE KF6:: )

install(FILES ${kiconthemes_version_header}
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KIdleTime COMPONENT Devel)

include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
