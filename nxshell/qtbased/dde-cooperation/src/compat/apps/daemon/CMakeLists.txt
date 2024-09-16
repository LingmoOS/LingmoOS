set(PROJ_NAME "cooperation-daemon")

configure_file(
    "../config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"
)

FILE(GLOB CPP_SRC
    *.h
    *.cpp
    ${CMAKE_SOURCE_DIR}/src/singleton/*.h
    ${CMAKE_SOURCE_DIR}/src/singleton/*.cpp
)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network)
if (CMAKE_SYSTEM MATCHES "Linux")
    find_package(Dtk${DTK_VERSION_MAJOR} COMPONENTS Widget REQUIRED)
else()
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
endif()

if (WIN32)
    # 文件描述信息
    set (PRODUCT_INFO "Deepin cooperation server")
    set (PRODUCT_NAME "cooperation")
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../version.rc.in ${CMAKE_CURRENT_BINARY_DIR}/version.rc)
    set (RC_FILES ${CMAKE_CURRENT_BINARY_DIR}/version.rc)
    set (MANIFEST ${CMAKE_CURRENT_SOURCE_DIR}/../app.manifest)
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${PROJ_NAME}
        MANUAL_FINALIZATION
        ${CPP_SRC}
        ${RC_FILES}
        ${MANIFEST}
    )
else()
    add_executable(${PROJ_NAME}
        ${CPP_SRC}
        ${RC_FILES}
        ${MANIFEST}
    )
endif()

if (WIN32)
    add_link_options(${PROJ_NAME} PRIVATE -mwindows)
endif()

target_include_directories(${PROJ_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_include_directories(${PROJ_NAME} PUBLIC ${COMPAT_ROOT_DIR}/apps)

target_link_libraries(${PROJ_NAME}
  PRIVATE
  Qt${QT_VERSION_MAJOR}::Core
  Qt${QT_VERSION_MAJOR}::Network
  deepin_cross::framework
)

if (CMAKE_SYSTEM MATCHES "Linux")
    target_link_libraries(${PROJ_NAME}
        PRIVATE
        Dtk${DTK_VERSION_MAJOR}::Widget
    )
else()
    target_link_libraries(${PROJ_NAME}
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
    )
endif()

if (DEFINED EXECUTABLE_INSTALL_DIR)
    install (TARGETS ${PROJ_NAME} DESTINATION ${EXECUTABLE_INSTALL_DIR})
else()
    install (TARGETS ${PROJ_NAME} DESTINATION bin)
endif()
