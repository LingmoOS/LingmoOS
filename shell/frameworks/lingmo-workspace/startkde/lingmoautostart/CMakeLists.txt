add_library(LingmoAutostart STATIC lingmoautostart.cpp)
set_property(TARGET LingmoAutostart PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(LingmoAutostart KF6::CoreAddons KF6::ConfigCore)
