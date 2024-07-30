add_executable(split-lingmo-svgs)

target_sources(split-lingmo-svgs PRIVATE
    split-lingmo-svgs.cpp
)

target_link_libraries(split-lingmo-svgs
PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Svg
    KF6::Archive
    KF6::CoreAddons
    KF6::Svg
)

