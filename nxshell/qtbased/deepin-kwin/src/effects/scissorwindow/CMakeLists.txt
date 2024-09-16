#######################################
# Effect

set(scissorwindow_SOURCES
    main.cpp
    scissorwindow.cpp
    scissor.qrc
)

kwin4_add_effect_module(kwin4_effect_scissorwindow ${scissorwindow_SOURCES})
target_link_libraries(kwin4_effect_scissorwindow PRIVATE
    kwineffects
    kwinglutils

    Qt${QT_MAJOR_VERSION}::Core
)
