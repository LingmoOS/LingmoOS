set(plasmoidviewer_SRCS
    main.cpp
    view.cpp
)

add_executable(plasmoidviewer ${plasmoidviewer_SRCS})
target_compile_definitions(plasmoidviewer PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries(plasmoidviewer
    Qt::Quick
    Qt::DBus
    KF6::DBusAddons
    Plasma::Plasma
    Plasma::PlasmaQuick
    KF6::I18n
    KF6::ConfigCore
    KF6::ConfigWidgets
    KF6::Package
)

install(TARGETS plasmoidviewer ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(PROGRAMS org.kde.plasmoidviewer.desktop  DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.plasmoidviewer.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

kdoctools_create_manpage (man-plasmoidviewer.1.docbook 1 INSTALL_DESTINATION ${KDE_INSTALL_MANDIR})
install(FILES completions/plasmoidviewer.zsh RENAME _plasmoidviewer DESTINATION ${KDE_INSTALL_ZSHAUTOCOMPLETEDIR})


#qml
plasma_install_package(qmlpackages/shell org.kde.plasma.plasmoidviewershell shells shell)
