cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "6.5.0") # handled by release scripts
set(KF_DEP_VERSION "6.4.0") # handled by release scripts
project(KSvg VERSION ${KF_VERSION})

# ECM setup
include(FeatureSummary)
find_package(ECM 6.4.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEGitCommitHooks)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)

include(ECMGenerateExportHeader)
include(ECMGenerateHeaders)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)
include(ECMQtDeclareLoggingCategory)
include(ECMAddQch)
include(KDEPackageAppTemplates)
include(ECMGenerateQmlTypes)
include(ECMMarkNonGuiExecutable)
include(ECMDeprecationSettings)
include(ECMQmlModule)

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

ecm_setup_version(PROJECT
                  VARIABLE_PREFIX KSVG
                  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/ksvg_version.h"
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6SvgConfigVersion.cmake"
                  SOVERSION 6)

################# now find all used packages #################

set (REQUIRED_QT_VERSION 6.5.0)

find_package(Qt6 ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE COMPONENTS Quick Gui Qml Svg QuickControls2)

find_package(KF6 ${KF_DEP_VERSION} REQUIRED
    COMPONENTS
        Archive        # svgz
        Config         # rects cache
        ColorScheme
        CoreAddons
        GuiAddons      # KImageCache
        KirigamiPlatform      # Kirigami.Theme
)

#########################################################################

ecm_set_disabled_deprecation_versions(
    QT 6.5
    KF 5.102
)

#add_definitions(-Wno-deprecated)

#########################################################################

option(BUILD_TOOLS "Build and install KSVG tools." OFF)

option(BUILD_COVERAGE "Build Plasma Frameworks with gcov support" OFF)

if(BUILD_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lgcov")
endif()

# make ksvg_version.h available
include_directories(${CMAKE_CURRENT_BINARY_DIR})

################# list the subdirectories #################
add_subdirectory(src)

if (BUILD_TESTING)
    add_subdirectory(autotests)
endif()

################ create PlasmaConfig.cmake and install it ###########################

# create a Config.cmake and a ConfigVersion.cmake file and install them

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6Svg")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF6Svg_QCH
        FILE KF6SvgQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF6SvgQchTargets.cmake\")")
endif()

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF6SvgConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6SvgConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
  PATH_VARS  CMAKE_INSTALL_PREFIX
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF6SvgConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6SvgConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF6SvgTargets
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE KF6SvgTargets.cmake
        NAMESPACE KF6::
        COMPONENT Devel)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ksvg_version.h
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KSvg COMPONENT Devel )

include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
