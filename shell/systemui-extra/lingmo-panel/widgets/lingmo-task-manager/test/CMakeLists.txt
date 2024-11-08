set(lingmoTaskManagerTest_SRCS
        main.cpp
        window-thumbnail-view.h
        window-thumbnail-view.cpp
        task-manager-view.h
        task-manager-view.cpp
        )
include_directories(../)

set(QRC_FILES qml/qml.qrc)
add_executable(lingmo-task-manager-test ${QRC_FILES} ${lingmoTaskManagerTest_SRCS})

target_link_libraries(lingmo-task-manager-test PRIVATE
        Qt${QT_MAJOR_VERSION}::Core
        Qt${QT_MAJOR_VERSION}::DBus
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::X11Extras
        lingmosdk-waylandhelper
        lingmo-task-manager
        KF5::WindowSystem
        )
