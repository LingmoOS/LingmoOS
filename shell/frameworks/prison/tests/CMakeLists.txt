add_executable(prison-test)
target_sources(prison-test PRIVATE prisontest.cpp barcodeexamplewidget.cpp main.cpp)
target_link_libraries(prison-test Qt6::Widgets KF6::Prison)
