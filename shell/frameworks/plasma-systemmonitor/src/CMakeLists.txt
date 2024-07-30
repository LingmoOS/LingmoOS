add_subdirectory(page)
add_subdirectory(table)
add_subdirectory(faces)


add_definitions(-DQT_NO_KEYWORDS)

add_executable(plasma-systemmonitor ${plasma-systemmonitor_SRCS} ${plasma-systemmonitor_QRC})
ecm_add_qml_module(plasma-systemmonitor URI "org.kde.systemmonitor" GENERATE_PLUGIN_SOURCE DEPENDENCIES "QtCore")

target_sources(plasma-systemmonitor PRIVATE
    main.cpp
    ToolsModel.cpp
    ToolsModel.h
    Configuration.cpp
    Configuration.h
    CommandLineArguments.cpp
    CommandLineArguments.h
)

ecm_target_qml_sources(plasma-systemmonitor SOURCES
    Main.qml
    GlobalMenu.qml
)

kconfig_target_kcfg_file(plasma-systemmonitor FILE systemmonitor.kcfg CLASS_NAME SystemMonitorConfiguration MUTATORS GENERATE_PROPERTIES GENERATE_MOC)

target_compile_definitions(plasma-systemmonitor PRIVATE PROJECT_VERSION="${PROJECT_VERSION}")
target_link_libraries(plasma-systemmonitor PRIVATE
    Qt::Quick
    Qt::Widgets
    Qt::DBus
    KF6::I18n
    KF6::ConfigGui
    KF6::Crash
    KF6::Service
    KF6::KIOGui
    KF6::GlobalAccel
    KF6::DBusAddons
    KF6::WindowSystem
    PlasmaSystemMonitorPage
    PlasmaSystemMonitorTable
)

install(TARGETS plasma-systemmonitor DESTINATION ${KDE_INSTALL_BINDIR})
install(FILES systemmonitor.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES plasma-systemmonitor.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})

install(FILES
    pages/applications.page
    pages/history.page
    pages/overview.page
    pages/processes.page
    DESTINATION ${KDE_INSTALL_DATADIR}/plasma-systemmonitor
)

ecm_finalize_qml_module(plasma-systemmonitor)
