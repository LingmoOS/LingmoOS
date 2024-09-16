find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets WebEngineWidgets REQUIRED)
# install qtwebengine5-dev / qt6-webengine-dev manually
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets WebEngineWidgets REQUIRED)

include_directories(
    global
)

link_libraries(
    ${Qt_LIBS}
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::WebEngineWidgets
)

set(LIB_NAME login)

aux_source_directory(. SRCS)

# add_library(${LIB_NAME} STATIC ${SRCS})

add_library(${LIB_NAME} SHARED ${SRCS})

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/modules)

install(TARGETS ${LIB_NAME} LIBRARY DESTINATION lib/dde-session-shell/modules)
