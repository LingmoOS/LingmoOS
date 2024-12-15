#######################################
# Effect

set(maximize_SOURCES
    main.cpp
    maximizeeffect.cpp
)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    QuickWidgets
)

kwin4_add_effect_module(kwin4_effect_maximizeex ${maximize_SOURCES})
target_link_libraries(kwin4_effect_maximizeex PRIVATE
    kwineffects
    Qt5::QuickWidgets
    Qt5::DBus
    Qt5::X11Extras
    KF5::ConfigGui
)
