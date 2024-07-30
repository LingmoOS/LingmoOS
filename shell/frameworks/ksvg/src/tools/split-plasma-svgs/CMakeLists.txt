add_executable(split-plasma-svgs)

target_sources(split-plasma-svgs PRIVATE
    split-plasma-svgs.cpp
)

target_link_libraries(split-plasma-svgs
PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Svg
    KF6::Archive
    KF6::CoreAddons
    KF6::Svg
)

