add_executable(examplelingmoshell)

target_sources(examplelingmoshell PRIVATE
    customcorona.cpp
    main.cpp
)

target_link_libraries(examplelingmoshell
    Qt6::Widgets
    Qt6::Quick
    Qt6::Qml
    KF6::I18n
    Lingmo::LingmoQuick
    Lingmo::Lingmo
    KF6::DBusAddons
    KF6::Notifications
    KF6::Package
)

