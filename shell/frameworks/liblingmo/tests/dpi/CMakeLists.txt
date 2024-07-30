add_executable(dpitest
    main.cpp
    dpitest.cpp
)

target_link_libraries(dpitest Lingmo::Lingmo KF6::I18n Qt6::Gui)

