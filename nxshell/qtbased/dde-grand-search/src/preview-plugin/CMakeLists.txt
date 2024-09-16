set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

set(FRONTEND_NAME dde-grand-search)

# 应用库目录
set(LIB_BASE_DIR "${LIB_PLATFORM_DIR}/${FRONTEND_NAME}")

# 插件目录
set(PLUGIN_BASE_DIR "${LIB_BASE_DIR}/plugins")

# 预览插件目录
set(PLUGIN_PREVIEW_DIR "${PLUGIN_BASE_DIR}/preview")
add_definitions(-DPLUGIN_PREVIEW_DIR="${PLUGIN_PREVIEW_DIR}")

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

set(QRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/resource.qrc
)

# 前端
set(FRONTEND_NAME dde-grand-search)

include_directories(${CMAKE_SOURCE_DIR}/src/grand-search/gui/exhibition/preview)

add_subdirectory(text-preview)
add_subdirectory(image-preview)
add_subdirectory(audio-preview)
add_subdirectory(pdf-preview)
add_subdirectory(video-preview)
