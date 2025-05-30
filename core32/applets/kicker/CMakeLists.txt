add_definitions(
  -DQT_USE_QSTRINGBUILDER
  -DQT_NO_CAST_TO_ASCII
  -DQT_NO_CAST_FROM_ASCII
  -DQT_STRICT_ITERATORS
  -DQT_NO_CAST_FROM_BYTEARRAY
  -DQT_USE_FAST_OPERATOR_PLUS
  -DTRANSLATION_DOMAIN=\"libkicker\"
)

set(kickerplugin_SRCS
    plugin/abstractentry.cpp
    plugin/abstractmodel.cpp
    plugin/actionlist.cpp
    plugin/appentry.cpp
    plugin/appsmodel.cpp
    plugin/computermodel.cpp
    plugin/contactentry.cpp
    plugin/containmentinterface.cpp
    plugin/draghelper.cpp
    plugin/simplefavoritesmodel.cpp
    plugin/kastatsfavoritesmodel.cpp
    plugin/fileentry.cpp
    plugin/forwardingmodel.cpp
    plugin/placeholdermodel.cpp
    plugin/funnelmodel.cpp
    plugin/dashboardwindow.cpp
    plugin/kickerplugin.cpp
    plugin/menuentryeditor.cpp
    plugin/processrunner.cpp
    plugin/rootmodel.cpp
    plugin/runnermodel.cpp
    plugin/runnermatchesmodel.cpp
    plugin/recentcontactsmodel.cpp
    plugin/recentusagemodel.cpp
    plugin/submenu.cpp
    plugin/systementry.cpp
    plugin/systemmodel.cpp
    plugin/systemsettings.cpp
    plugin/wheelinterceptor.cpp
    plugin/windowsystem.cpp
    plugin/funnelmodel.cpp
    plugin/kickercompattrianglemousefilter.cpp
    ../../components/trianglemousefilter/trianglemousefilter.cpp
)

ecm_qt_declare_logging_category(kickerplugin_SRCS
    HEADER debug.h
    IDENTIFIER KICKER_DEBUG
    CATEGORY_NAME org.kde.plasma.kicker)

qt_add_dbus_interface(kickerplugin_SRCS ${CMAKE_SOURCE_DIR}/krunner/dbus/org.kde.krunner.App.xml krunner_interface)
qt_add_dbus_interface(kickerplugin_SRCS ${CMAKE_SOURCE_DIR}/ksmserver/org.kde.KSMServerInterface.xml ksmserver_interface)

install(FILES plugin/qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/kicker)

add_library(kickerplugin SHARED ${kickerplugin_SRCS})

if (BUILD_TESTING)
    set_target_properties(kickerplugin PROPERTIES CXX_VISIBILITY_PRESET default)
endif()

target_link_libraries(kickerplugin
                      Qt::Core
                      Qt::Qml
                      Qt::Quick
                      KF5::Activities
                      KF5::ActivitiesStats
                      KF5::ConfigCore
                      KF5::CoreAddons
                      KF5::I18n
                      KF5::IconThemes
                      KF5::ItemModels
                      KF5::KIOCore
                      KF5::KIOWidgets
                      KF5::KIOFileWidgets
                      KF5::Notifications
                      KF5::People
                      KF5::PeopleWidgets
                      KF5::PlasmaQuick
                      KF5::Runner
                      KF5::Service
                      KF5::WindowSystem
                      PW::KWorkspace)

if (${HAVE_APPSTREAMQT})
target_link_libraries(kickerplugin AppStreamQt)
endif()

if (${HAVE_ICU})
    target_link_libraries(kickerplugin ICU::i18n ICU::uc)
    target_compile_definitions(kickerplugin PRIVATE "-DHAVE_ICU")
endif()

if (BUILD_TESTING)
    add_subdirectory(plugin/autotests)
endif()

install(TARGETS kickerplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/kicker)
