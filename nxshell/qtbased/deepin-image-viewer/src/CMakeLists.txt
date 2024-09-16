cmake_minimum_required(VERSION 3.13)

# 玲珑构建依赖目录
include_directories(${CMAKE_INSTALL_PREFIX}/include)
include_directories(${CMAKE_INSTALL_PREFIX}/include/deepin-ocr-plugin-manager)

set(APP_BIN_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/bin/)
set(BIN_NAME ${CMAKE_PROJECT_NAME})
list(APPEND RESOURCES)

find_package(Qt${QT_VERSION_MAJOR} CONFIG REQUIRED COMPONENTS
    Quick
    DBus
    Concurrent
    Svg
    PrintSupport
    LinguistTools
)

find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS
    Widget
    Declarative
)

# OCR
find_package(PkgConfig REQUIRED)
pkg_search_module(OCR_PLUGIN REQUIRED deepin-ocr-plugin-manager)
include_directories(${OCR_PLUGIN_INCLUDE_DIRS})

# 保证 src 目录下头文件全局可见
include_directories(src)

# 源文件
file(GLOB_RECURSE SRCS CONFIGURE_DEPENDS "./*.h" "./*.cpp")
file(GLOB_RECURSE QMLSRC ./*.qml)

if(NOT BUILD_WITH_QT6)
    find_package(Qt5QuickCompiler CONFIG REQUIRED)
    qtquick_compiler_add_resources(RCC_SOURCES ${RESOURCES})
endif()

if(NOT(${CMAKE_BUILD_TYPE} MATCHES "Debug"))
    # 调整优化级别
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

    # Translation
    file(GLOB TS LIST_DIRECTORIES false translations/${CMAKE_PROJECT_NAME}*.ts)
    set_source_files_properties(${TS} PROPERTIES OUTPUT_LOCATION ${PROJECT_SOURCE_DIR}/translations)
    qt_create_translation(QM ${SRCS} ${QMLSRC} ${TS})
endif()

# 仅生成 EXE 文件，用以兼容新(5.6.0+dev-1及以上)旧两版本DtkDeclarative库
add_executable(${BIN_NAME}
    ${SRCS}
    ${RCC_SOURCES}
    ${QM}
    deepin-image-viewer.qrc
    res.qrc
)

target_include_directories(${BIN_NAME} PUBLIC ${3rd_lib_INCLUDE_DIRS})
target_link_libraries(${BIN_NAME}
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::PrintSupport
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Svg

    # 移除
    Dtk${DTK_VERSION_MAJOR}::Widget
    Dtk${DTK_VERSION_MAJOR}::Declarative
    GL
    pthread
    freeimage
    ${OCR_PLUGIN_LIBRARIES}
)

if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
    TARGET_COMPILE_DEFINITIONS(${BIN_NAME} PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
endif()

set(APPSHAREDIR ${CMAKE_INSTALL_PREFIX}/share/deepin-image-viewer)

# Add install rule
install(TARGETS ${BIN_NAME} DESTINATION ${APP_BIN_INSTALL_DIR})
install(FILES ${CMAKE_PROJECT_NAME}.desktop DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)
install(FILES ./icon/${CMAKE_PROJECT_NAME}.svg DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps)
install(FILES com.deepin.imageViewer.service DESTINATION ${CMAKE_INSTALL_PREFIX}/share/dbus-1/services/)

# translations
install(DIRECTORY ${PROJECT_SOURCE_DIR}/translations
    DESTINATION ${APPSHAREDIR}
    FILES_MATCHING PATTERN "*.qm")

# manual
install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/assets/deepin-image-viewer
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/deepin-manual/manual-assets/application/
    FILES_MATCHING PATTERN "*")
