# Project name
project(HelloInternet)
find_package(Qt6 COMPONENTS Core Network Xml Widgets REQUIRED)
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
# Add the SequenceDemo executable and link the ThreadWeaver framework:
add_executable(ThreadWeaver_HelloInternet
    main.cpp
    MainWidget.cpp
    ViewController.cpp
    HelloInternetResources.qrc
)

kde_enable_exceptions()
target_link_libraries(ThreadWeaver_HelloInternet Qt6::Network Qt6::Xml Qt6::Widgets KF6::ThreadWeaver)

