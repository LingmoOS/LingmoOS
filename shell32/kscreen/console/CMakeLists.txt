add_definitions(-DTRANSLATION_DOMAIN=\"kscreen\")

add_executable(kscreen-console main.cpp console.cpp)

target_link_libraries(kscreen-console
            Qt::DBus
            Qt::Gui
            KF5::CoreAddons
            KF5::I18n
            KF5::Screen)

install(TARGETS kscreen-console ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
