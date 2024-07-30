project(nanoshellprivate)


add_library(plasmananoshellprivateplugin SHARED)
target_sources(plasmananoshellprivateplugin PRIVATE
    fullscreenoverlay.cpp
    fullscreenoverlay.h
    nanoshellprivateplugin.cpp
    nanoshellprivateplugin.h
    startupnotifier.cpp
    startupnotifier.h
    )
target_link_libraries(plasmananoshellprivateplugin Qt::Core Qt::Qml Qt::Quick  KF6::WindowSystem Plasma::KWaylandClient KF6::Service)

install(TARGETS plasmananoshellprivateplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/nanoshell)

install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/nanoshell)

install(DIRECTORY qml DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/nanoshell/)


