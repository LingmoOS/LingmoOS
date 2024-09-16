# 定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

# 设置cmake参数
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 设置工程名字
project(drawboard-example)

# 设置Qt模块
set(QtModule Core Gui Widgets DBus PrintSupport Svg Concurrent LinguistTools)

# 查找Qt相关的库链接
find_package(Qt5 REQUIRED ${QtModule})

include_directories(${BASE_LIB_INCLUDEDIRS})

set(EXAMPLESRCDIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

# 包含源码文件夹并获取其下所有源代码文件
foreach(item IN LISTS EXAMPLESRCDIRS)
    include_directories(${item})
    list(APPEND EXAMPLE_SRC_CPPS    ${item}/*.cpp)
    list(APPEND EXAMPLE_SRC_HEADERS ${item}/*.h)
    list(APPEND EXAMPLE_SRC_QRCS    ${item}/*.qrc)
endforeach()
FILE (GLOB allExampleSource  ${EXAMPLE_SRC_CPPS})
FILE (GLOB allExampleHeaders ${EXAMPLE_SRC_HEADERS})
FILE (GLOB allExampleQrcs    ${EXAMPLE_SRC_QRCS})

add_executable(${PROJECT_NAME} ${allExampleSource} ${allExampleHeaders} ${allExampleQrcs})

target_include_directories(${PROJECT_NAME} PUBLIC ${3rd_lib_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} ${3rd_lib_LIBRARIES} ${CMAKE_DL_LIBS} ${BASE_LIB})

# 将工程与Qt模块链接起来
qt5_use_modules(${PROJECT_NAME} ${QtModule})





