cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "6.5.0") # handled by release scripts
set(KF_DEP_VERSION "6.4.0") # handled by release scripts
project(KFileMetaData VERSION ${KF_VERSION})

include(FeatureSummary)
find_package(ECM 6.4.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(ECMAddTests)
include(ECMGenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMAddQch)
include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)
include(CheckStructHasMember)

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

set(kfilemetadata_version_header "${CMAKE_CURRENT_BINARY_DIR}/src/kfilemetadata_version.h")
ecm_setup_version(PROJECT
                  VARIABLE_PREFIX KFILEMETADATA
                  VERSION_HEADER "${kfilemetadata_version_header}"
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6FileMetaDataConfigVersion.cmake"
                  SOVERSION 3)

# Dependencies
set(REQUIRED_QT_VERSION 6.5.0)

find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG OPTIONAL_COMPONENTS Gui)
set_package_properties(Qt6Gui PROPERTIES DESCRIPTION "Qt6: Gui component"
                       TYPE OPTIONAL
                       PURPOSE "Qt6::Gui is needed for several extractors")

find_package(Qt6 ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE COMPONENTS Xml)

find_package(KF6 ${KF_DEP_VERSION} COMPONENTS Archive)
set_package_properties(KF6Archive PROPERTIES DESCRIPTION "KDE Frameworks: Archive Framework"
                       URL "https://download.kde.org/stable/frameworks/" TYPE OPTIONAL
                       PURPOSE "Archive is needed for several extractors")

find_package(KF6 ${KF_DEP_VERSION} COMPONENTS CoreAddons)
set_package_properties(KF6CoreAddons PROPERTIES DESCRIPTION "KDE Frameworks: Core Addons Framework"
                       URL "https://download.kde.org/stable/frameworks/" TYPE REQUIRED
                       PURPOSE "Needed for the formatting of properties for display purposes")

find_package(KF6 ${KF_DEP_VERSION} COMPONENTS Config)
set_package_properties(KF6Config PROPERTIES DESCRIPTION "KDE Frameworks: Config Framework"
                       URL "https://download.kde.org/stable/frameworks/" TYPE OPTIONAL
                       PURPOSE "Config is needed to build the AppImage extractor")

find_package(KF6 ${KF_DEP_VERSION} REQUIRED COMPONENTS I18n)

find_package(KF6 ${KF_DEP_VERSION} COMPONENTS Codecs)
set_package_properties(KF6Codecs PROPERTIES DESCRIPTION "KDE Frameworks: KCodecs Framework"
                       URL "https://download.kde.org/stable/frameworks/" TYPE REQUIRED
                       PURPOSE "Needed for the autodetecting encoding of plain text files")

find_package(Poppler 0.12.1 COMPONENTS Qt6)
set_package_properties(Poppler PROPERTIES DESCRIPTION "A PDF rendering library"
                       URL "https://poppler.freedesktop.org/" TYPE OPTIONAL
                       PURPOSE "Support for PDF files")

find_package(Taglib 1.12)
set_package_properties(Taglib PROPERTIES
                       TYPE OPTIONAL
                       PURPOSE "Support for music metadata")

find_package(LibExiv2 0.26)
set_package_properties(LibExiv2 PROPERTIES
                       TYPE OPTIONAL
                       PURPOSE "Support for image metadata")

find_package(FFmpeg 57.48 COMPONENTS AVCODEC)
if (AVCODEC_FOUND)
    find_package(FFmpeg 57.40 COMPONENTS AVFORMAT)
    if (AVFORMAT_FOUND)
        find_package(FFmpeg 55.27 COMPONENTS AVUTIL)
    endif()
endif()
set_package_properties(FFmpeg PROPERTIES DESCRIPTION "Video Tag reader"
                       URL "https://ffmpeg.org/" TYPE OPTIONAL
                       PURPOSE "Support for video metadata")

find_package(EPub)
set_package_properties(EPub PROPERTIES DESCRIPTION "Ebook epub reader"
                       URL "https://sourceforge.net/projects/ebook-tools/" TYPE OPTIONAL
                       PURPOSE "Support for epub metadata")

find_package(CatDoc)
set_package_properties(CatDoc PROPERTIES DESCRIPTION "catdoc executable"
                       URL "https://www.wagner.pp.ru/~vitus/software/catdoc/" TYPE RUNTIME
                       PURPOSE "Extract text from office 98 files - RUNTIME dependency")

if ( CMAKE_SYSTEM_NAME MATCHES "Linux" )
    find_package(Xattr)
    set_package_properties(Xattr PROPERTIES DESCRIPTION "library libattr "
        URL "https://savannah.nongnu.org/projects/attr"
        TYPE REQUIRED
        PURPOSE "Extended attribute shared library")
endif()

find_package(QMobipocket6 2.1)
set_package_properties(QMobipocket6 PROPERTIES DESCRIPTION "Mobipocket epub reader"
                       URL "https://invent.kde.org/graphics/kdegraphics-mobipocket"
                       TYPE OPTIONAL PURPOSE "Support for mobi metadata")

find_package(libappimage 0.1.10 CONFIG)
set_package_properties(libappimage PROPERTIES DESCRIPTION "Core library of the AppImage project"
                       URL "https://github.com/AppImage/libappimage"
                       TYPE OPTIONAL
                       PURPOSE "Needed to build the AppImage extractor"
                       )

ecm_set_disabled_deprecation_versions(
    QT 6.4
    KF 5.102
)

add_definitions(-DTRANSLATION_DOMAIN=\"kfilemetadata6\")
add_subdirectory(src)
if (BUILD_TESTING)
  add_subdirectory(autotests)
  add_subdirectory(tests)
endif()

# Config files
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6FileMetaData")

ki18n_install(po)

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF6FileMetaData_QCH
        FILE KF6FileMetaDataQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF6FileMetaDataQchTargets.cmake\")")
endif()

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF6FileMetaDataConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6FileMetaDataConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF6FileMetaDataConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6FileMetaDataConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT devel
)

install(FILES
  ${kfilemetadata_version_header}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KFileMetaData COMPONENT Devel
)

include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
