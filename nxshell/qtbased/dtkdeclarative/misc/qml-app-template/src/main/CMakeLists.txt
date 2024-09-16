cmake_minimum_required(VERSION 3.1)

set(APP_BIN_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/bin/)
set(APP_NAME ${CMAKE_PROJECT_NAME})
add_definitions(-DAPP_NAME="${APP_NAME}")

###! main目录创建出二进制可执行文件，但启动方式由 DAppLoader
###! 中提供, 请不要在 main 中实现额外内容，请将需要实现的部分
###! 移动到 preload 插件和 maincomponent 插件中。

# Set sources
set(SRC
    main.cpp
)

# Find the QtQuick library
find_package(Qt${QT_VERSION_MAJOR}Quick CONFIG REQUIRED)

# Add EXE
add_executable(${APP_NAME} ${SRC})

# 由于 EXE 只是作为启动程序加载 LIB 使用，一般来说，只需链接 ${LIB_NAME} 既可。
target_link_libraries(${APP_NAME}
    Qt${QT_VERSION_MAJOR}::Quick
    Dtk${DTK_VERSION_MAJOR}::Declarative
)

add_definitions(-DAPP_PLUGIN_PATH=\"${DTK_QML_APP_PLUGIN_PATH}\")

set_target_properties(${APP_NAME} PROPERTIES INSTALL_RPATH ${DTK_QML_APP_PLUGIN_PATH})
install(TARGETS ${APP_NAME} DESTINATION ${APP_BIN_INSTALL_DIR})
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/../../${CMAKE_PROJECT_NAME}.desktop" DESTINATION share/applications)
