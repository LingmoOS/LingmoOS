plasma_install_package(package org.kde.plasma.timer)

ecm_add_qml_module(timerplugin URI org.kde.plasma.private.timer)
target_sources(timerplugin PRIVATE plugin/timerplugin.cpp)
target_link_libraries(timerplugin PRIVATE Qt::Core Qt::Qml)
ecm_finalize_qml_module(timerplugin)

install(FILES timer.svgz DESTINATION ${PLASMA_DATA_INSTALL_DIR}/desktoptheme/default/widgets/)
install(FILES plasma_applet_timer.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR})
