cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "6.5.0")
project(KHolidays VERSION ${KF_VERSION})

find_package(ECM 6.4.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
set(CMAKE_AUTORCC ON)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)

include(GenerateExportHeader)
include(ECMGenerateHeaders)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)
include(ECMPoQmTools)
include(FeatureSummary)
include(ECMCoverageOption)
include(ECMQtDeclareLoggingCategory)
include(ECMAddQch)
include(KDEGitCommitHooks)
include(ECMGenerateExportHeader)
include(ECMDeprecationSettings)
include(ECMQmlModule)

set(KHOLIDAYS_LIB_VERSION ${KF_VERSION})
set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")

set(kholidays_version_header "${CMAKE_CURRENT_BINARY_DIR}/src/kholidays_version.h")
ecm_setup_version(PROJECT VARIABLE_PREFIX KHOLIDAYS
                        VERSION_HEADER "${kholidays_version_header}"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6HolidaysConfigVersion.cmake"
                        SOVERSION 6
)

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

########### Find packages ###########
set(REQUIRED_QT_VERSION 6.5.0)
find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Core Qml)

remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII)
ecm_set_disabled_deprecation_versions(
    QT 6.5
)

########### Targets ###########
add_subdirectory(src)

if(BUILD_TESTING)
  add_subdirectory(autotests)
endif()

ecm_install_po_files_as_qm(poqm)

########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6Holidays")

if(BUILD_QCH)
  ecm_install_qch_export(
    TARGETS KF6Holidays_QCH
    FILE KF6HolidaysQchTargets.cmake
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    COMPONENT Devel
  )
  set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF6HolidaysQchTargets.cmake\")")
endif()

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF6HolidaysConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6HolidaysConfig.cmake"
  INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF6HolidaysConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6HolidaysConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF6HolidaysTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF6HolidaysTargets.cmake NAMESPACE KF6::)

install(FILES
  ${kholidays_version_header}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KHolidays COMPONENT Devel
)

include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
