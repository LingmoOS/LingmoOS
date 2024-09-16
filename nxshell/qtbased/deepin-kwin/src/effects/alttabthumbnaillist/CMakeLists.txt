#######################################
# Effect

# Source files
set(alttabthumbnaillist_SOURCES
    alttabthumbnaillist.cpp
    alttabthumbnaillist.qrc
    main.cpp
)
kwin4_add_effect_module(kwin4_effect_alttabthumbnaillist ${alttabthumbnaillist_SOURCES})
target_link_libraries(kwin4_effect_alttabthumbnaillist PRIVATE
    kwineffects
    kwinglutils
)
