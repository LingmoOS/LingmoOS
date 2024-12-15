set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core DBus)

add_executable(unionid-app main.cpp app.cpp app.h)
target_link_libraries(unionid-app Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::DBus)
