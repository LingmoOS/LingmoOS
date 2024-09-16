# 定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIE -pie")

# 定义可执行程序名称
set(BIN_NAME ${DAEMON_BIN_NAME})

#定义库目录
add_definitions(-DDAEMON_LIB_BASE_DIR="${DAEMON_LIB_BASE_DIR}")

# 集成测试标签
ADD_DEFINITIONS(-DENABLE_ACCESSIBILITY)

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
    DBus
REQUIRED)

set(Qt_LIBS
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::DBus
    Qt5::Concurrent
)

pkg_check_modules(DFrameworkDBus REQUIRED dframeworkdbus)

# 源文件
set(SRCS
    main.cpp
    daemonlibrary.cpp
    daemonlibrary.h
)

add_executable(${BIN_NAME} ${SRCS})

target_include_directories(${BIN_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${DtkWidget_INCLUDE_DIRS}
    ${DtkGUI_INCLUDE_DIRS}
    ${DFrameworkDBus_INCLUDE_DIRS}
)

target_link_libraries(${BIN_NAME} PRIVATE
    ${Qt_LIBS}
    ${DtkWidget_LIBRARIES}
    ${DtkGUI_LIBRARIES}
    ${DFrameworkDBus_LIBRARIES}
)

# Install settings
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

# bin
install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})

# dbus
install(FILES data/com.deepin.dde.daemon.GrandSearch.service DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services)

install(FILES data/dde-grand-search-daemon.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
