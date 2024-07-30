find_package(Qt6Qml REQUIRED)

add_executable(kglobalacceltest kglobalacceltest.cpp)
target_link_libraries(kglobalacceltest KF6::GlobalAccel Qt6::Qml Qt6::Test)
