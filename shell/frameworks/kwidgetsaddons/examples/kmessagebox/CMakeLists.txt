
add_executable(kwidgetaddons-example-kmessagebox main.cpp)

target_link_libraries(kwidgetaddons-example-kmessagebox
    Qt6::Widgets
    KF6::WidgetsAddons
)

# We don't want to install the tutorial
#install(TARGETS kwidgetaddons-example-kmessagebox  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
