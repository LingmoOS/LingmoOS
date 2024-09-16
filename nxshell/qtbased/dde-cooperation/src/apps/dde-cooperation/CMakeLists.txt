set(PROJ_NAME "dde-cooperation")
set(PKG_NAME "dde-cooperation")

# enable qt build support
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

configure_file(
    "../config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/config.h"
)

FILE(GLOB CPP_SRC
    *.h
    *.cpp
    ${CMAKE_SOURCE_DIR}/src/singleton/*.h
    ${CMAKE_SOURCE_DIR}/src/singleton/*.cpp
    ${CMAKE_SOURCE_DIR}/src/base/*.h
    ${CMAKE_SOURCE_DIR}/src/base/*.cpp
)

set(qt_required_components Core Gui Network Widgets)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS ${qt_required_components})
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS ${qt_required_components})
if (CMAKE_SYSTEM MATCHES "Linux")
    find_package(Dtk${DTK_VERSION_MAJOR} COMPONENTS Widget REQUIRED)
endif()

#如果是Windows平台，则生成rc文件，还有inno setup脚本文件
set(VERSION_RC_PATH "")
set(MANIFEST "")
if (WIN32)
    # 文件描述信息
    set (PRODUCT_INFO "Deepin cooperation")
    set (PRODUCT_NAME "deepin-cooperation")
    set (PRODUCT_ID "A3AE65FC-2431-49AE-9A9C-87D3DBC2B7A4")

    set(VERSION_RC_PATH ${CMAKE_CURRENT_BINARY_DIR}/version_${PROJ_NAME}.rc)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../version.rc.in ${VERSION_RC_PATH})

    set(MANIFEST ${CMAKE_CURRENT_SOURCE_DIR}/../app.manifest)

    # 从定位输出目录
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${DEPLOY_OUTPUT_DIRECTORY}/${PKG_NAME})

    # 安装包制作inno脚本
    configure_file(${CMAKE_SOURCE_DIR}/dist/inno/setup.iss.in ${CMAKE_BINARY_DIR}/${PROJ_NAME}-setup.iss)
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${PROJ_NAME}
        MANUAL_FINALIZATION
        ${CPP_SRC}
        ${VERSION_RC_PATH}
        ${MANIFEST}
    )
else()
    add_executable(${PROJ_NAME}
        ${CPP_SRC}
        ${VERSION_RC_PATH}
        ${MANIFEST}
    )
endif()

# 使用定义的辅助程序执行目录
target_compile_definitions(${PROJECT_NAME} PRIVATE COMPAT_DAEMON_DIR="${EXECUTABLE_INSTALL_DIR}")

target_include_directories(${PROJ_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_include_directories(${PROJ_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/src/lib/cooperation)

if (WIN32)
    # add_link_options(${PROJ_NAME} PRIVATE -mwindows)
    #设置属性
    set_target_properties(${PROJ_NAME} PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER www.deepin.org
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
    )
endif()

target_link_libraries(${PROJ_NAME}
  PRIVATE
  Qt${QT_VERSION_MAJOR}::Core
  Qt${QT_VERSION_MAJOR}::Gui
  Qt${QT_VERSION_MAJOR}::Network
  Qt${QT_VERSION_MAJOR}::Widgets
  cooperation-core
)

if (CMAKE_SYSTEM MATCHES "Linux")
    target_link_libraries(${PROJ_NAME}
        PRIVATE
        Dtk${DTK_VERSION_MAJOR}::Widget
    )
    include(install_dconfig)
    INSTALL_DCONFIG("org.deepin.dde.cooperation.json")
endif()

install(TARGETS ${PROJ_NAME} DESTINATION bin)

# translation files
TRANSLATION_GENERATE(QM_FILES ${CMAKE_SOURCE_DIR}/translations/${PROJ_NAME})
add_custom_target(${PROJ_NAME}_qm_files DEPENDS ${QM_FILES})
add_dependencies(${PROJ_NAME} ${PROJ_NAME}_qm_files)

if (WIN32)
    # 添加一个自定义构建步骤，在构建后立即拷贝翻译文件到目标目录
    add_custom_command(TARGET ${PROJ_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}/translations
        COMMAND ${CMAKE_COMMAND} -E copy ${QM_FILES} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}/translations
        COMMENT "Copying translation files to output directory"
    )

    deployqt_qt_runtime(${PROJ_NAME} ${qt_required_components})
else()
    # translation
    install(FILES ${QM_FILES} DESTINATION share/${PROJ_NAME}/translations)

    # icons
    FILE(GLOB ICON_FILES ${CMAKE_CURRENT_SOURCE_DIR}/res/linux/*.svg)
    install(FILES ${ICON_FILES} DESTINATION share/icons/hicolor/scalable/apps)

    # desktop
    install(FILES res/linux/dde-cooperation.desktop DESTINATION share/applications)

    # manual
    install(DIRECTORY assets/dde-cooperation DESTINATION share/deepin-manual/manual-assets/application)

    # 日志收集配置
    install(FILES res/linux/dde-cooperation.json DESTINATION share/deepin-log-viewer/deepin-log.conf.d)
endif()
