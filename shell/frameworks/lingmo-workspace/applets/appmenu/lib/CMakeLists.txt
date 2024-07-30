kcoreaddons_add_plugin(org.kde.lingmo.appmenu SOURCES appmenuapplet.cpp INSTALL_NAMESPACE "lingmo/applets")

target_link_libraries(org.kde.lingmo.appmenu
                      Qt::Widgets
                      Qt::Quick
                      Qt::DBus
                      Lingmo::Lingmo
                      KF6::WindowSystem
                      PW::LibTaskManager)
