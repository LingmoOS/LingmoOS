add_executable(printconfig testplugandplay.cpp testpnp.cpp)
target_link_libraries(printconfig Qt::Gui KF6::Screen)

add_subdirectory(kwayland)
