add_definitions(-DTRANSLATION_DOMAIN=\"lingmowindowed\")

set(lingmowindowed-app_SRCS
    lingmowindowedcorona.cpp
    lingmowindowedview.cpp
    lingmowindowedcorona.h
    lingmowindowedview.h
    main.cpp
)

add_executable(lingmowindowed ${lingmowindowed-app_SRCS})

target_link_libraries(
    lingmowindowed
    Qt::Widgets
    Qt::Quick
    Qt::Qml
    KF6::I18n
    KF6::IconThemes
    KF6::XmlGui
    Lingmo::LingmoQuick
    Lingmo::Lingmo
    KF6::DBusAddons
    KF6::Notifications
    KF6::Package
    KF6::StatusNotifierItem
)

install(TARGETS lingmowindowed ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
#even if hidden, the desktop file is needed anyways for kdbusservice::unique
configure_file(org.kde.lingmowindowed.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/org.kde.lingmowindowed.desktop @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.lingmowindowed.desktop DESTINATION ${KDE_INSTALL_APPDIR})
