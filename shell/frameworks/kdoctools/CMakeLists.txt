cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "6.5.0") # handled by release scripts
set(KF_DEP_VERSION "6.4.0") # handled by release scripts
project(KDocTools VERSION ${KF_VERSION})

# ECM setup
include(FeatureSummary)
find_package(ECM 6.4.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)

include(KDEGitCommitHooks)
include(ECMSetupVersion)
include(ECMMarkNonGuiExecutable)
include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)
include(CMakePackageConfigHelpers)
include(ECMGenerateExportHeader)
include(ECMAddQch)

set(kdoctools_version_header "${CMAKE_CURRENT_BINARY_DIR}/src/kdoctools_version.h")
ecm_setup_version(PROJECT VARIABLE_PREFIX KDOCTOOLS
                        VERSION_HEADER "${kdoctools_version_header}"
                        PACKAGE_VERSION_FILE "${CMAKE_BINARY_DIR}/cmake/KF6DocToolsConfigVersion.cmake"
                        SOVERSION 6)

option(MEINPROC_NO_KARCHIVE "Compile meinproc without KArchive dependency. Only for internal usage on KDE infrastructure." OFF)
option(INSTALL_INTERNAL_TOOLS "Install internal tools too." OFF)
option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

# Dependencies
set(REQUIRED_QT_VERSION 6.5.0)
find_package(Qt6Core ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)

# KF6 frameworks
if (NOT MEINPROC_NO_KARCHIVE)
   find_package(KF6Archive ${KF_DEP_VERSION} REQUIRED)
else()
   message(WARNING "KArchive dependency has been disabled. This should be done only for internal usage on KDE infrastructure.")
endif()

find_package(KF6I18n ${KF_DEP_VERSION})
set_package_properties(KF6I18n PROPERTIES TYPE OPTIONAL
   PURPOSE "Required to build the KDocTools translations")

find_package(LibXslt)
set_package_properties(LibXslt PROPERTIES
                       URL "http://xmlsoft.org/XSLT"
                       TYPE REQUIRED
                       PURPOSE "Required by the KDE help system to process DocBook XML"
                      )

find_package(LibXml2)
set_package_properties(LibXml2 PROPERTIES
                       URL "http://xmlsoft.org"
                       TYPE REQUIRED
                       PURPOSE "Required by the KDE help system to process DocBook XML"
                      )

if (NOT LIBXML2_XMLLINT_EXECUTABLE)
   message(FATAL_ERROR "xmllint is required to process DocBook XML")
endif()

find_package(DocBookXML4 "4.5")

set_package_properties(DocBookXML4 PROPERTIES
                       TYPE REQUIRED
                       PURPOSE "Required by the KDE help system to process DocBook XML"
                      )

find_package(DocBookXSL)
set_package_properties(DocBookXSL PROPERTIES
                       TYPE REQUIRED
                       PURPOSE "Required by the KDE help system to process DocBook XML"
                      )

remove_definitions(-DQT_NO_CAST_FROM_ASCII)
set(_kdoctoolsBootStrapping TRUE)
set(KDOCTOOLS_CUSTOMIZATION_DIR "${CMAKE_CURRENT_BINARY_DIR}/src/customization/")
if (WIN32)
    set(KDOCTOOLS_MEINPROC_EXECUTABLE          ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/meinproc6 )
else ()
    set(KDOCTOOLS_MEINPROC_EXECUTABLE          ${CMAKE_CURRENT_BINARY_DIR}/src/meinproc6${CMAKE_EXECUTABLE_SUFFIX} )
endif ()
include(KF6DocToolsMacros.cmake)

set(KDOCTOOLS_LANGUAGES_LIST af bg ca ca@valencia cs da de el en-GB en eo es et fi fo fr gl he hu id it ja ko lt nds nl nn no pl pt-BR pt ro ru sk sl sq sr sr@latin sr@ijekavian sr@ijekavianlatin sv th tr uk wa xh xx zh-CN zh-TW)

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6DocTools")
add_definitions(-DTRANSLATION_DOMAIN=\"kdoctools6\")
ecm_set_disabled_deprecation_versions(
    QT 6.5
    KF 5.102
)

if (KF6I18n_FOUND)
    ki18n_install(po)
    kdoctools_install(po)
endif()
add_subdirectory(common)
add_subdirectory(docs)
add_subdirectory(src)
if (BUILD_TESTING)
    add_subdirectory(autotests)
endif()

install(EXPORT KF6DocToolsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF6DocToolsTargets.cmake NAMESPACE KF6:: )
install(EXPORT KF6DocToolsToolsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF6DocToolsToolsTargets.cmake NAMESPACE KF6:: )
install(
    FILES ${kdoctools_version_header}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KDocTools
    COMPONENT Devel
)

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF6DocTools_QCH
        FILE "${CMAKE_BINARY_DIR}/cmake/KF6DocToolsQchTargets.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF6DocToolsQchTargets.cmake\")")
endif()

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF6DocToolsConfig.cmake.in"
  "${CMAKE_BINARY_DIR}/cmake/KF6DocToolsConfig.cmake"
  PATH_VARS
    KDE_INSTALL_DATADIR_KF
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/KF6DocToolsMacros.cmake ${CMAKE_BINARY_DIR}/cmake/KF6DocToolsMacros.cmake COPYONLY)
# we need a separate copy with local paths for install autotests
export(EXPORT KF6DocToolsTargets FILE ${CMAKE_BINARY_DIR}/cmake/KF6DocToolsTargets.cmake NAMESPACE KF6::)
export(EXPORT KF6DocToolsToolsTargets FILE ${CMAKE_BINARY_DIR}/cmake/KF6DocToolsToolsTargets.cmake NAMESPACE KF6::)

install(FILES
  "${CMAKE_BINARY_DIR}/cmake/KF6DocToolsConfig.cmake"
  "${CMAKE_BINARY_DIR}/cmake/KF6DocToolsConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

configure_file(config-kdoctools.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kdoctools.h )

install( FILES
    KF6DocToolsMacros.cmake
    DESTINATION ${CMAKECONFIG_INSTALL_DIR} COMPONENT Devel )

include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
