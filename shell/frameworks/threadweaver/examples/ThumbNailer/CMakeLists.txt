# Project name
project(ThumbNailer)
kde_enable_exceptions()
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
qt_wrap_ui(UI_FILES MainWindow.ui)
add_executable(ThreadWeaver_ThumbNailer
    ThumbNailer.cpp
    Image.cpp
    Model.cpp
    MainWindow.cpp
    Benchmark.cpp
    PriorityDecorator.cpp
    ItemDelegate.cpp
    ImageListFilter.cpp
    FileLoaderJob.cpp
    ImageLoaderJob.cpp
    ComputeThumbNailJob.cpp
    AverageLoadManager.cpp
    ${UI_FILES}
)
target_link_libraries(ThreadWeaver_ThumbNailer KF6::ThreadWeaver Qt6::Test Qt6::Widgets)
