set(sendfilehelper_SRCS
    discoverwidget.cpp
    main.cpp
    sendfilesjob.cpp
    sendfilewizard.cpp

    pages/connectingpage.cpp
    pages/failpage.cpp
    pages/selectdeviceandfilespage.cpp
    pages/selectdevicepage.cpp
    pages/selectfilespage.cpp
)

ecm_qt_declare_logging_category(sendfilehelper_SRCS
    HEADER bluedevil_sendfile.h
    IDENTIFIER BLUEDEVIL_SENDFILE_LOG
    CATEGORY_NAME org.kde.lingmo.bluedevil.sendfile
    DESCRIPTION "BlueDevil (send file)"
    EXPORT BLUEDEVIL
)

ki18n_wrap_ui(sendfilehelper_SRCS
    discover.ui
    pages/connecting.ui
    pages/failpage.ui
    pages/selectfilediscover.ui
)

add_executable(bluedevil-sendfile ${sendfilehelper_SRCS})

target_link_libraries(bluedevil-sendfile
    Qt::DBus
    Qt::Widgets

    KF6::BluezQt
    KF6::CoreAddons
    KF6::DBusAddons
    KF6::I18n
    KF6::KIOFileWidgets
    KF6::WidgetsAddons
    KF6::WindowSystem
)

install(TARGETS bluedevil-sendfile DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES org.kde.bluedevilsendfile.desktop DESTINATION ${KDE_INSTALL_APPDIR})
