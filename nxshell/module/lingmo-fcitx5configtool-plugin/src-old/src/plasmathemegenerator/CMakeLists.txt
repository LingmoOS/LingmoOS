add_executable(fcitx5-plasma-theme-generator main.cpp)

target_link_libraries(fcitx5-plasma-theme-generator KF5::Plasma KF5::IconThemes KF5::I18n Fcitx5::Utils Fcitx5::Config)

install(TARGETS fcitx5-plasma-theme-generator DESTINATION ${CMAKE_INSTALL_BINDIR})
