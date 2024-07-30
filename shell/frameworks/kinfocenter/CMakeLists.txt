project(kinfocenter)
set(PROJECT_VERSION "6.1.80")

cmake_minimum_required(VERSION 3.16)
set(QT_MIN_VERSION "6.6.0")

set(KF6_MIN_VERSION "6.2.0")
find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH}  ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)
include(KDEClangFormat)
include(ECMFindQmlModule)
include(KDEGitCommitHooks)
include(ECMSetupQtPluginMacroNames)
include(ECMQueryQt)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Gui Widgets)

find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS
    Config
    CoreAddons
    DocTools
    I18n
    KCMUtils
    KIO
    Service
    Solid
    Auth
)

ecm_find_qmlmodule(org.kde.lingmoui 2.5)

qt_add_resources(RESOURCES resources.qrc)

macro(kinfocenter_add_kcm target)
    kcmutils_add_qml_kcm(${target} DISABLE_DESKTOP_FILE_GENERATION SOURCES ${ARGN} INSTALL_NAMESPACE lingmo/kcms/kinfocenter)
endmacro()

find_package(SystemSettings QUIET)
set_package_properties(SystemSettings PROPERTIES
    DESCRIPTION "Lingmo's SystemSettings"
    TYPE RUNTIME
    PURPOSE "Absolutely required because the KInfoCenter binary is a symlink to it!"
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_FROM_BYTEARRAY -DQT_NO_KEYWORDS)
add_definitions(-DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT)
add_definitions(-DQT_NO_URL_CAST_FROM_STRING)
add_definitions(-DQT_USE_QSTRINGBUILDER)

ecm_query_qt(QtBinariesDir QT_INSTALL_BINS)

add_subdirectory( appiumtests )
add_subdirectory( categories )
add_subdirectory( kcontrol/menus )
add_subdirectory( kcms )
add_subdirectory(src)

include(ECMOptionalAddSubdirectory)
ecm_optional_add_subdirectory( doc )
kdoctools_install(po)

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

##install

install( FILES org.kde.kinfocenter.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install( PROGRAMS org.kde.kinfocenter.desktop  DESTINATION  ${KDE_INSTALL_APPDIR} )

ADD_CUSTOM_TARGET(kinfocenter_link ALL
                  COMMAND ${CMAKE_COMMAND} -E create_symlink ${KDE_INSTALL_FULL_BINDIR}/systemsettings ${CMAKE_CURRENT_BINARY_DIR}/kinfocenter)

install( FILES ${CMAKE_CURRENT_BINARY_DIR}/kinfocenter DESTINATION ${KDE_INSTALL_FULL_BINDIR}/ )

ki18n_install(po)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
