#cmake 最小版本要求
cmake_minimum_required(VERSION 3.1.0)

#工程名
project(downloader)
if (CVERSION)
  message("CVERSION: ${CVERSION}")
  set(PROJECT_VERSION ${CVERSION})
else ()
  set(PROJECT_VERSION "5.3.70")
endif ()
set(PROJECT_VERSION_MAJOR 5)
set(QT_MIN_VERSION "5.11.0")
set(DLM_NAME "downloader")
set(ARIA2_NAME "aria2c")


# 设置C++标准为 C++ 11
set(CMAKE_CXX_STANDARD 11)

#在debug下库名后面加d
set(CMAKE_DEBUG_POSTFIX "d")

#设置安装路径
set(CMAKE_INSTALL_PREFIX /usr)
## Install settings
#if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
#    set(CMAKE_INSTALL_PREFIX /usr)
#endif ()

#option(DOTEST "option for test" OFF)

if (NOT (${CMAKE_BUILD_TYPE} MATCHES "Debug"))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")
    message("release")
    # generate qm
    execute_process(COMMAND bash "translate_generation.sh"
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif ()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector-all -z now -z noexecstack -fpie -pie")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all -z now -z noexecstack -fpie -pie")

set(APP_RES_DIR "assets")
if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "mips64")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wl,-O1 -Wl,--as-needed -Wl,-E -fPIE -ffunction-sections -fdata-sections -Wl,--gc-sections -ftree-vectorize -march=loongson3a -mhard-float -mno-micromips -mno-mips16 -flax-vector-conversions -mloongson-ext2 -mloongson-mmi -z noexecstack")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -Wl,-O1 -Wl,--as-needed -Wl,-E -fPIE -ffunction-sections -fdata-sections -Wl,--gc-sections -ftree-vectorize -march=loongson3a -mhard-float -mno-micromips -mno-mips16 -flax-vector-conversions -mloongson-ext2 -mloongson-mmi -z noexecstack")
    SET(CMAKE_EXE_LINKER_FLAGS  "-pie")
endif()
#库 和 执行文件 的设置输出的路径
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin)

configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )

message(STATUS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

#包含当前路径，使用*.ui文件时，需要加上这句，否则找不到头文件
set(CMAKE_INCLUDE_CURRENT_DIR ON)
#打开全局moc
set(CMAKE_AUTOMOC ON)
#打开全局rcc，本示例中没有使用qrc
set(CMAKE_AUTORCC ON)
#打开全局uic
set(CMAKE_AUTOUIC ON)

set(QT Core Gui Widgets Network DBus Sql LinguistTools Svg Test WebChannel WebSockets)
find_package(Qt5 COMPONENTS ${QT} REQUIRED)

#应用DTK库
find_package(DtkWidget REQUIRED)
find_package(DtkCore REQUIRED)
find_package(DtkGui REQUIRED)

#安装
# qm files.
file(GLOB QM_FILES "translations/*.qm")
install(FILES ${QM_FILES} DESTINATION share/downloader/translations)

#desktop
install(FILES desktop/downloader.desktop DESTINATION share/applications)

#datebases
install(FILES data/downloader.db DESTINATION share/downloader/database)

# conf file
file(GLOB CF_FILES "docs/*.conf")
install(FILES ${CF_FILES} DESTINATION share/downloader/config)
file(GLOB JS_FILES "docs/httpAdvanced.json")
install(FILES ${JS_FILES} DESTINATION share/downloader/config)

#icon
install(FILES data/downloader.svg DESTINATION share/icons/hicolor/scalable/apps/)
install(FILES data/downloader.svg DESTINATION share/downloader/icons/logo/)

# 开启单元测试

# 添加子模块test
add_subdirectory(src)
add_subdirectory(translations)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
add_subdirectory(tests)
endif()

install(TARGETS downloader dlmlog dlmdatabase dlmaria2 dlmcontrolui
RUNTIME DESTINATION bin
LIBRARY DESTINATION lib
ARCHIVE DESTINATION libstatic
)

if (USE_AM_DBUS)
set(SERVICE_SCRIPT docs/dbus/startdlmservice.sh)
else()
set(SERVICE_SCRIPT docs/nodbus/startdlmservice.sh)
endif()

install(DIRECTORY ${APP_RES_DIR}/downloader DESTINATION /usr/share/deepin-manual/manual-assets/application/)
install(FILES docs/info.json DESTINATION share/downloader/extension/)
install(FILES docs/ojlicckikdkkaclkpdddijgehekpmmbg.crx DESTINATION share/downloader/extension/)
install(FILES docs/browser.downloader.autostart.json DESTINATION /etc/browser/native-messaging-hosts/)
install(FILES ${SERVICE_SCRIPT} DESTINATION /usr/libexec/openconnect/)
