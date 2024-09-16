set(plugin_SRCS
    previewbutton.cpp
    previewbridge.cpp
    previewclient.cpp
    previewitem.cpp
    previewsettings.cpp
    plugin.cpp
    buttonsmodel.cpp
    ../../../decorations/decorationpalette.cpp
    ../../../decorations/decorations_logging.cpp
)

add_library(kdecorationprivatedeclarative SHARED ${plugin_SRCS})
target_link_libraries(kdecorationprivatedeclarative
    KDecoration2::KDecoration
    KDecoration2::KDecoration2Private
    Qt${QT_MAJOR_VERSION}::DBus
    Qt${QT_MAJOR_VERSION}::Quick
    KF5::CoreAddons
    KF5::ConfigWidgets
    KF5::I18n
    KF5::Service
)

install(TARGETS kdecorationprivatedeclarative DESTINATION ${KDE_INSTALL_QMLDIR}/org/deepin/kwin/private/kdecoration )
install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/deepin/kwin/private/kdecoration )
