cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "6.5.0") # handled by release scripts
set(KF_DEP_VERSION "6.4.0") # handled by release scripts
project(KTextWidgets VERSION ${KF_VERSION})

# ECM setup
include(FeatureSummary)
find_package(ECM 6.4.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(KDEGitCommitHooks)

include(ECMGenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(CMakePackageConfigHelpers)
include(ECMAddQch)
include(ECMDeprecationSettings)
include(CMakeDependentOption)

set(ktextwidgets_version_header "${CMAKE_CURRENT_BINARY_DIR}/src/ktextwidgets_version.h")
ecm_setup_version(PROJECT
                  VARIABLE_PREFIX KTEXTWIDGETS
                  VERSION_HEADER "${ktextwidgets_version_header}"
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6TextWidgetsConfigVersion.cmake"
                  SOVERSION 6)

# Dependencies
set(REQUIRED_QT_VERSION 6.5.0)

option(WITH_TEXT_TO_SPEECH "Build text-to-speech support" ON)

find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Widgets)

if (WITH_TEXT_TO_SPEECH)
    find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED TextToSpeech)
    add_definitions(-DHAVE_SPEECH)
endif()

find_package(KF6Completion ${KF_DEP_VERSION} REQUIRED)
find_package(KF6Config ${KF_DEP_VERSION} REQUIRED)
find_package(KF6ConfigWidgets ${KF_DEP_VERSION} REQUIRED)
find_package(KF6I18n ${KF_DEP_VERSION} REQUIRED)
find_package(KF6WidgetsAddons ${KF_DEP_VERSION} REQUIRED)
find_package(KF6Sonnet ${KF_DEP_VERSION} REQUIRED)

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

cmake_dependent_option(BUILD_DESIGNERPLUGIN "Build plugin for Qt Designer" ON "NOT CMAKE_CROSSCOMPILING" OFF)
add_feature_info(DESIGNERPLUGIN ${BUILD_DESIGNERPLUGIN} "Build plugin for Qt Designer")

add_definitions(-DTRANSLATION_DOMAIN=\"ktextwidgets6\")

ecm_set_disabled_deprecation_versions(
    QT 6.5
    KF 5.240
)

ki18n_install(po)
add_subdirectory(src)
if (BUILD_TESTING)
    add_subdirectory(tests)
    add_subdirectory(autotests)
endif()

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6TextWidgets")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF6TextWidgets_QCH
        FILE KF6TextWidgetsQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF6TextWidgetsQchTargets.cmake\")")
endif()

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KF6TextWidgetsConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KF6TextWidgetsConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                             )

install(EXPORT KF6TextWidgetsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF6TextWidgetsTargets.cmake NAMESPACE KF6:: )

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/KF6TextWidgetsConfig.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/KF6TextWidgetsConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
       )

install(FILES ${ktextwidgets_version_header}
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KTextWidgets COMPONENT Devel
       )

include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
