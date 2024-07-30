add_definitions(-DTRANSLATION_DOMAIN=\"kscreen\")

add_executable(kscreen-console main.cpp console.cpp)

target_link_libraries(kscreen-console
            Qt::DBus
            Qt::Gui
            KF6::CoreAddons
            KF6::I18n
            KF6::Screen)

install(TARGETS kscreen-console ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
