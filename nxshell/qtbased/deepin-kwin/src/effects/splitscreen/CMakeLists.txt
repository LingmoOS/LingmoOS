#######################################
# Effect

set(splitscreen_SOURCES
    main.cpp
    splitprevieweffect.cpp
)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    QuickWidgets
)

kwin4_add_effect_module(kwin4_effect_splitscreen ${splitscreen_SOURCES})
target_link_libraries(kwin4_effect_splitscreen PRIVATE
    kwineffects
    Qt5::QuickWidgets
    Qt5::DBus
    Qt5::X11Extras
    KF5::ConfigGui
)

install(DIRECTORY qml DESTINATION ${KDE_INSTALL_DATADIR}/deepin-kwin/effects/splitscreen)
