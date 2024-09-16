# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.

# SPDX-License-Identifier: CC0-1.0
set(PRELOAD_LIB_NAME ${CMAKE_PROJECT_NAME}-preload)
set(PRELOAD_SRC
    preloadplugin.cpp
    )
find_package(Qt5QuickCompiler REQUIRED)
qtquick_compiler_add_resources(PRELOAD_RCC_SOURCES preload.qrc)
include_directories(src/preloadplugin)
add_library(${PRELOAD_LIB_NAME} SHARED
    ${PRELOAD_SRC}
    ${PRELOAD_RCC_SOURCES}
    )
target_link_libraries(${PRELOAD_LIB_NAME}
    Qt5::Quick
    ${DtkDeclarative_LIBRARIES}
    )
install(TARGETS ${PRELOAD_LIB_NAME} DESTINATION ${LIB_INSTALL_DIR})
