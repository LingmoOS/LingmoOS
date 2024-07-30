cmake_minimum_required(VERSION 3.16)

project(kmenuedit)
set(PROJECT_VERSION "6.1.80")

set(QT_MIN_VERSION "6.6.0")
set(KF6_MIN_VERSION "6.2.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} )

include(FeatureSummary)
include(ECMAddAppIcon)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMInstallIcons)
include(ECMQtDeclareLoggingCategory)
include(KDEClangFormat)
include(KDEGitCommitHooks)
include(ECMDeprecationSettings)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    DBus
    Xml
)

find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS
    I18n
    XmlGui
    DBusAddons
    IconThemes
    KIO
    ItemViews
    Sonnet
    DocTools
    GlobalAccel
    WindowSystem
)

ecm_set_disabled_deprecation_versions(QT 6.4
     KF 5.240.0
    )

#add_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_STRICT_ITERATORS -DQT_NO_CAST_FROM_BYTEARRAY)

add_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_FROM_BYTEARRAY)
add_definitions(-DTRANSLATION_DOMAIN=\"kmenuedit\")
add_subdirectory( pixmaps ) 
add_subdirectory( doc )

set(kmenueditcommon_STAT_SRCS basictab.cpp
    kmenuedit.qrc
    basictab.h
    configurationmanager.cpp
    configurationmanager.h
    globalaccel.cpp
    globalaccel.h
    klinespellchecking.cpp
    klinespellchecking.h
    kmenuedit.cpp
    kmenuedit.h
    main.cpp
    menufile.cpp
    menufile.h
    menuinfo.cpp
    menuinfo.h
    preferencesdlg.cpp
    preferencesdlg.h
    treeview.cpp
    treeview.h)
########### next target ###############
qt_add_dbus_adaptor( kmenueditcommon_STAT_SRCS org.kde.kmenuedit.xml kmenuedit.h KMenuEdit)

ecm_qt_declare_logging_category(kmenueditcommon_STAT_SRCS HEADER kmenuedit_debug.h IDENTIFIER KMENUEDIT_LOG CATEGORY_NAME org.kde.kmenuedit DESCRIPTION "kmenuedit" EXPORT KMENUEDIT)

set(kmenuedit_KDEINIT_SRCS main.cpp ${kmenueditcommon_STAT_SRCS})

add_executable( kmenuedit ${kmenuedit_KDEINIT_SRCS})
target_compile_definitions(kmenuedit PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries(kmenuedit
    Qt::DBus
    Qt::Xml
    KF6::DBusAddons
    KF6::GlobalAccel
    KF6::I18n
    KF6::IconThemes
    KF6::KIOCore
    KF6::KIOWidgets
    KF6::ItemViews
    KF6::SonnetCore
    KF6::SonnetUi
    KF6::XmlGui
    KF6::WindowSystem
    KF6::IconWidgets
)

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
install(TARGETS kmenuedit ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install( PROGRAMS org.kde.kmenuedit.desktop  DESTINATION  ${KDE_INSTALL_APPDIR} )
install( FILES org.kde.kmenuedit.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR} )

ecm_install_icons( ICONS 16-apps-kmenuedit.png  22-apps-kmenuedit.png  32-apps-kmenuedit.png  48-apps-kmenuedit.png DESTINATION ${KDE_INSTALL_ICONDIR} THEME hicolor  )
feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

ecm_qt_install_logging_categories(
        EXPORT KMENUEDIT
        FILE kmenuedit.categories
        DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
        )

ki18n_install(po)
kdoctools_install(po)
