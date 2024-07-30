add_subdirectory(tool)

add_executable(menudemo menudemo.cpp)
target_link_libraries(menudemo KF6::PurposeWidgets)
