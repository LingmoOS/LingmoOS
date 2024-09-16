# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.

# SPDX-License-Identifier: CC0-1.0
set(APP_NAME ${CMAKE_PROJECT_NAME})
add_definitions(-DAPP_NAME="${APP_NAME}")

###! main目录创建出二进制可执行文件，但启动方式由 DAppLoader
###! 中提供, 请不要在 main 中实现额外内容，请将需要实现的部分
###! 移动到 preload 插件和 maincomponent 插件中。

# Set sources
set(SRC
    main.cpp
    )

# Find the Qt5Quick library

# Add EXE
add_executable(${APP_NAME} ${SRC} ${QRC})

# 由于 EXE 只是作为启动程序加载 LIB 使用，一般来说，只需链接 ${LIB_NAME} 既可。
target_link_libraries(${APP_NAME}
    Qt5::Quick
    ${DtkDeclarative_LIBRARIES}
    )


install(TARGETS ${APP_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
