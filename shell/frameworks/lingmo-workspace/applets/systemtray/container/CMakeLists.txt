
lingmo_install_package(package org.kde.lingmo.systemtray)

set(systemtraycontainer_SRCS
    systemtraycontainer.cpp
    systemtraycontainer.h
)

ecm_qt_declare_logging_category(systemtraycontainer_SRCS HEADER debug.h
                                            IDENTIFIER SYSTEM_TRAY_CONTAINER
                                            CATEGORY_NAME kde.systemtraycontainer
                                            DEFAULT_SEVERITY Info
                                            DESCRIPTION "lingmo applet systemtray container"
                                            EXPORT LINGMOWORKSPACE
                                        )

kcoreaddons_add_plugin(org.kde.lingmo.systemtray SOURCES ${systemtraycontainer_SRCS} INSTALL_NAMESPACE "lingmo/applets")

target_link_libraries(org.kde.lingmo.systemtray
                      Qt::Gui
                      Qt::Quick
                      Lingmo::Lingmo
                      Lingmo::LingmoQuick
                      KF6::XmlGui
                      KF6::I18n)
