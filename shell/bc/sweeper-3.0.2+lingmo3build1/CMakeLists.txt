cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

# KDE Application Version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "22")
set (RELEASE_SERVICE_VERSION_MINOR "12")
set (RELEASE_SERVICE_VERSION_MICRO "3")
set (RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(sweeper VERSION ${RELEASE_SERVICE_VERSION})

set(QT_MIN_VERSION "5.15.2")
set(KF5_MIN_VERSION "5.90.0")

find_package(ECM ${KF5_MIN_VERSION} NO_MODULE REQUIRED)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMAddAppIcon)
include(ECMQtDeclareLoggingCategory)
include(FeatureSummary)

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} REQUIRED COMPONENTS Core DBus Widgets)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    ActivitiesStats
    Bookmarks
    Crash
    Config
    ConfigWidgets
    CoreAddons
    I18n
    KIO
    TextWidgets
    XmlGui
)
find_package(KF5DocTools ${KF5_MIN_VERSION})
set_package_properties(KF5DocTools PROPERTIES DESCRIPTION
    "Tools to generate documentation"
    TYPE OPTIONAL
)


add_definitions(
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050f00
    -DQT_DEPRECATED_WARNINGS_SINCE=0x060000
    -DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x055800
    -DKF_DEPRECATED_WARNINGS_SINCE=0x060000
)

include_directories (${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

configure_file(config-sweeper.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config-sweeper.h
)

add_executable(sweeper)
target_sources(sweeper PRIVATE
   main.cpp 
   sweeper.cpp 
   privacyfunctions.cpp 
   privacyaction.cpp
   privacyaction.h
   sweeper.h
   privacyfunctions.h
   )
ecm_qt_declare_logging_category(sweeper HEADER sweeper_debug.h IDENTIFIER SWEEPER_LOG CATEGORY_NAME org.kde.sweeper DESCRIPTION "sweeper" EXPORT SWEEPER)


qt_add_dbus_adaptor( sweeper_SRCS org.kde.sweeper.xml sweeper.h Sweeper)
target_sources(sweeper PRIVATE ${sweeper_SRCS})
		
ki18n_wrap_ui(sweeper sweeperdialog.ui )

target_link_libraries(sweeper
   Qt${QT_MAJOR_VERSION}::Core
   Qt${QT_MAJOR_VERSION}::DBus
   Qt${QT_MAJOR_VERSION}::Widgets
   KF5::ActivitiesStats
   KF5::Bookmarks
   KF5::Crash
   KF5::I18n
   KF5::KIOCore
   KF5::TextWidgets
   KF5::XmlGui
)

install(TARGETS sweeper  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( PROGRAMS org.kde.sweeper.desktop  DESTINATION ${KDE_INSTALL_APPDIR})
install( FILES sweeperui.rc  DESTINATION ${KDE_INSTALL_KXMLGUIDIR}/sweeper)
install( FILES org.kde.sweeper.xml DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR} )
install( FILES org.kde.sweeper.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

ki18n_install(po)
if(KF5DocTools_FOUND)
    kdoctools_install(po)
    add_subdirectory(doc)
endif()
ecm_qt_install_logging_categories(
        EXPORT SWEEPER
        FILE sweeper.categories
        DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
        )

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
