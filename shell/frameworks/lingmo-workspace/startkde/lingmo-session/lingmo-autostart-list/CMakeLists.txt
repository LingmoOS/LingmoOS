add_executable(lingmo-autostart-list main.cpp ../autostart.cpp)
target_link_libraries(lingmo-autostart-list KF6::Service LingmoAutostart)
