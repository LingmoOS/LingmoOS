
set(BIN_NAME dde-wloutput-daemon)

# 增加安全编译参数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector-all")
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -z relro -z now -z noexecstack -pie")

set(TARGET_NAME dde-wloutput-daemon)
FILE(GLOB SC_FILES "*.cpp" "*.h")

add_executable(${TARGET_NAME} ${SC_FILES})

# Find the library
find_package(PkgConfig REQUIRED)
find_package(DtkCore REQUIRED)
find_package(Qt5 COMPONENTS
    Core
    Gui
    DBus
    Widgets
REQUIRED)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
find_package(DWayland)

target_link_libraries(${BIN_NAME}
    Qt5::Core
    Qt5::DBus
    Qt5::Widgets
    ${DtkCore_LIBRARIES}
    DWaylandClient
)

install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
