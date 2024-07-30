remove_definitions(-DQT_NO_CAST_FROM_ASCII)

add_executable(kplottest)
target_sources(kplottest PRIVATE testplot_widget.cpp testplot_main.cpp)

target_link_libraries(kplottest KF6::Plotting Qt6::Widgets)
