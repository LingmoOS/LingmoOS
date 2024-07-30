cmake_minimum_required(VERSION 3.16)
set(KF_VERSION "6.5.0") # handled by release scripts
set(KF_DEP_VERSION "6.4.0") # handled by release scripts
project(KUserFeedback VERSION ${KF_VERSION})

# ECM setup
include(FeatureSummary)
find_package(ECM ${KF_DEP_VERSION}  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://invent.kde.org/frameworks/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ ${ECM_MODULE_PATH} ${CMAKE_MODULE_PATH})

set(ENABLE_SURVEY_TARGET_EXPRESSIONS ON CACHE BOOL "Survey targeting expressions support is enabled by default.")
set(ENABLE_PHP                       ON CACHE BOOL "Syntax checking of PHP server code.")
set(ENABLE_PHP_UNIT                  ON CACHE BOOL "Unit tests for PHP server code.")
set(ENABLE_DOCS                      ON CACHE BOOL "Generate documentation.")
set(ENABLE_CONSOLE                   ON CACHE BOOL "Application console.")
set(ENABLE_CLI                       ON CACHE BOOL "Command line interface support.")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)

include(ECMGenerateHeaders)
include(ECMPoQmTools)
include(ECMSetupVersion)
include(ECMQueryQt)
include(ECMGeneratePriFile)
include(ECMQmlModule)
include(FeatureSummary)
include(GenerateExportHeader)

include(ECMQtDeclareLoggingCategory)

ecm_setup_version(PROJECT
    VARIABLE_PREFIX KUSERFEEDBACK
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kuserfeedback_version.h"
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6UserFeedbackConfigVersion.cmake"
    SOVERSION 6
)
#
# Dependencies
#

# Dependencies
set(REQUIRED_QT_VERSION 6.5.0)
find_package(Qt6 ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED COMPONENTS Core Network)
find_package(Qt6 ${REQUIRED_QT_VERSION} NO_MODULE QUIET OPTIONAL_COMPONENTS Widgets Charts Test Qml Svg PrintSupport)
find_package(QmlLint)

if (ENABLE_DOCS)
    if (NOT CMAKE_CROSSCOMPILING)
        find_package(Qt6 NO_MODULE QUIET OPTIONAL_COMPONENTS Help)
    endif()

    find_package(Qt6 CONFIG OPTIONAL_COMPONENTS ToolsTools)
endif()

set_package_properties(Qt6 PROPERTIES URL "http://qt-project.org/")
set_package_properties(Qt6Widgets PROPERTIES TYPE RECOMMENDED PURPOSE "Required for feedback configuration and notification widgets.")
set_package_properties(Qt6Charts PROPERTIES TYPE RECOMMENDED PURPOSE "Required for UserFeedbackConsole.")

# debug suffixes for qmake compatibility
if(WIN32)
    set(CMAKE_DEBUG_POSTFIX "d")
elseif(APPLE)
    set(CMAKE_DEBUG_POSTFIX "_debug")
endif()

add_definitions(-DQT_USE_QSTRINGBUILDER -DQT_USE_FAST_OPERATOR_PLUS -DQT_NO_CAST_TO_ASCII -DQT_NO_URL_CAST_FROM_STRING -DQT_NO_CAST_FROM_ASCII)

remove_definitions(-DQT_NO_FOREACH) # still uses foreach
remove_definitions(-DQT_NO_KEYWORDS)

if (ENABLE_SURVEY_TARGET_EXPRESSIONS)
    find_package(FLEX)
    set_package_properties(FLEX PROPERTIES TYPE RECOMMENDED PURPOSE "Survey target expression parser.")

    find_package(BISON)
    set_package_properties(BISON PROPERTIES TYPE RECOMMENDED PURPOSE "Survey target expression parser.")

    if(FLEX_FOUND AND BISON_FOUND)
        set(HAVE_SURVEY_TARGET_EXPRESSIONS 1)
    endif()
endif()
add_feature_info("Survey targeting expressions support" HAVE_SURVEY_TARGET_EXPRESSIONS "Requires flex and bison parser generators.")

if (ENABLE_PHP)
    find_package(Php)
    set_package_properties(Php PROPERTIES URL "http://php.net" TYPE RECOMMENDED PURPOSE "Syntax checking of PHP server code.")
endif()

if (ENABLE_PHP_UNIT)
    find_package(PhpUnit)
    set_package_properties(PhpUnit PROPERTIES URL "http://phpunit.de" TYPE RECOMMENDED PURPOSE "Unit tests for PHP server code.")
endif()

#
# Actually build the stuff
#
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
configure_file(config-userfeedback.h.in ${CMAKE_CURRENT_BINARY_DIR}/config-userfeedback.h)

add_subdirectory(src)

if (BUILD_TESTING)
    add_subdirectory(autotests)
    add_subdirectory(tests)
endif()

if (ENABLE_DOCS)
    add_subdirectory(docs)
endif()

#
# CMake package config file generation
#
include(CMakePackageConfigHelpers)

set(_CONFIG_CMAKE_IN "KF6UserFeedbackConfig.cmake.in")

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${_CONFIG_CMAKE_IN}
    ${CMAKE_CURRENT_BINARY_DIR}/KF6UserFeedbackConfig.cmake
    INSTALL_DESTINATION ${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6UserFeedback
    PATH_VARS KDE_INSTALL_INCLUDEDIR
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/KF6UserFeedbackConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/KF6UserFeedbackConfigVersion.cmake
    DESTINATION ${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6UserFeedback
)

install(EXPORT KF6UserFeedbackTargets DESTINATION ${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6UserFeedback FILE KF6UserFeedbackTarget.cmake NAMESPACE KF6::)

ecm_qt_install_logging_categories(
        EXPORT KUSERFEEDBACK
        FILE org_kde_UserFeedback.categories
        DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
        )

ecm_install_po_files_as_qm(poqm)

include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

install(FILES
    ${KF6UserFeedbackCore_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/kuserfeedback_version.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KUserFeedback
)
