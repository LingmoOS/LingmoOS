project(nanoshellprivate)


add_library(lingmonanoshellprivateplugin SHARED)
target_sources(lingmonanoshellprivateplugin PRIVATE
    fullscreenoverlay.cpp
    fullscreenoverlay.h
    nanoshellprivateplugin.cpp
    nanoshellprivateplugin.h
    startupnotifier.cpp
    startupnotifier.h
    )
target_link_libraries(lingmonanoshellprivateplugin Qt::Core Qt::Qml Qt::Quick  KF6::WindowSystem Lingmo::KWaylandClient KF6::Service)

install(TARGETS lingmonanoshellprivateplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/lingmo/private/nanoshell)

install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/lingmo/private/nanoshell)

install(DIRECTORY qml DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/lingmo/private/nanoshell/)


