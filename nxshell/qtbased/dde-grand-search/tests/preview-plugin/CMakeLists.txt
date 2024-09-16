# 定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wl,--as-need -fPIE")

# 定义可执行程序名称
set(BIN_NAME test-preview-plugin)

# 依赖包
find_package(PkgConfig REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(DtkGui REQUIRED)
find_package(DtkCMake REQUIRED)
find_package(Qt5 COMPONENTS
    Core
    Gui
    Widgets
    Concurrent
REQUIRED)

set(Qt_LIBS
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Concurrent
)

# audio
find_package(PkgConfig REQUIRED)
pkg_check_modules(TAGLIB REQUIRED taglib)
pkg_check_modules(AVFORMAT REQUIRED libavformat)
pkg_check_modules(ICU REQUIRED icu-i18n)

# pdf
pkg_check_modules(PDF REQUIRED deepin-pdfium)
pkg_check_modules(JPEG REQUIRED libjpeg)

#video
pkg_check_modules(Ffm REQUIRED libffmpegthumbnailer)

set(PROJECT_SRC_PATH "${PROJECT_SOURCE_PATH}/preview-plugin")
include_directories(${PROJECT_SRC_PATH}
    ${PROJECT_SOURCE_PATH}/grand-search/gui/exhibition/preview)

set(QRCS
    ${PROJECT_SRC_PATH}/resource.qrc
)

#项目工程文件
FILE(GLOB_RECURSE PRO_SRC "${PROJECT_SRC_PATH}/*/*.h" "${PROJECT_SRC_PATH}/*/*.cpp")

# 移除Interface文件
FILE(GLOB_RECURSE RM_SRC
    "${PROJECT_SRC_PATH}/*/*interface.h" "${PROJECT_SRC_PATH}/*/*interface.cpp"
)

list(REMOVE_ITEM PRO_SRC ${RM_SRC})

#单元测试文件
FILE(GLOB_RECURSE UT_SRC "./*/*.cpp")

# 源文件
set(SRCS
    main.cpp
    ${GLOBAL_SRC}
    ${PRO_SRC}
    ${UT_SRC}
    ${CPP_STUB_SRC}
)

add_executable(${BIN_NAME} ${SRCS} ${QRCS})

target_include_directories(${BIN_NAME} PUBLIC
    ${DtkWidget_INCLUDE_DIRS}
    ${DtkGUI_INCLUDE_DIRS}
    ${PDF_INCLUDE_DIRS}
    ${TAGLIB_INCLUDE_DIRS}
    ${AVFORMAT_INCLUDE_DIRS}
    ${Avformat_INCLUDE_DIRS}
    ${Ffm_INCLUDE_DIRS}
)

target_link_libraries(${BIN_NAME} PRIVATE
    ${Qt_LIBS}
    ${DtkWidget_LIBRARIES}
    ${DtkGUI_LIBRARIES}
    ${GTEST_LIBRARIES}
    ${Qt5Test_LIBRARIES}
    ${TAGLIB_LIBRARIES}
    ${AVFORMAT_LIBRARIES}
    ${ICU_LIBRARIES}
    ${PDF_LIBRARIES}
    ${JPEG_LIBRARIES}
    ${Avformat_LIBRARIES}
    ${Ffm_LIBRARIES}
    -lpthread
)


