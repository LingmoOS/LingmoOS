find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets WebEngineWidgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets WebEngineWidgets REQUIRED)

include_directories(
    global
)

link_libraries(
    ${Qt_LIBS}
)

set(LIB_NAME webview)

aux_source_directory(. SRCS)

# add_library(${LIB_NAME} STATIC ${SRCS})

add_library(${LIB_NAME} SHARED ${SRCS})

target_link_libraries(${LIB_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::WebEngineWidgets
)

install(TARGETS ${LIB_NAME} LIBRARY DESTINATION lib/dde-session-shell/modules)
