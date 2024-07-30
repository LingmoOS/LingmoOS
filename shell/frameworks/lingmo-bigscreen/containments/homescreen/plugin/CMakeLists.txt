kde_enable_exceptions()

add_definitions(-DTRANSLATION_DOMAIN=\"biglauncherhomescreen\")

kcoreaddons_add_plugin(org.kde.mycroft.bigscreen.homescreen INSTALL_NAMESPACE "lingmo/applets")
target_sources(org.kde.mycroft.bigscreen.homescreen PRIVATE
    biglauncherhomescreen.cpp
    biglauncher_dbus.cpp
    applicationlistmodel.cpp
    kcmslistmodel.cpp
    configuration.cpp
)

target_link_libraries(org.kde.mycroft.bigscreen.homescreen
    Qt::Gui
    Qt::Core
    Qt::Qml
    Qt::DBus
    Qt::Network
    Lingmo::Lingmo
    KF6::I18n
    KF6::Notifications
    KF6::Service KF6::KIOWidgets
    Lingmo::Activities
    PW::KWorkspace
    Lingmo::ActivitiesStats
)
