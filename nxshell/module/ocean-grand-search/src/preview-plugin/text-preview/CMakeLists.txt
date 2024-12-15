project(text-preview)

# 定义插件名称
set(LIB_NAME text-preview-plugin)

set(SRCS
    textpreview_global.h
    textpreviewinterface.h
    textpreviewinterface.cpp
    textpreviewplugin.h
    textpreviewplugin.cpp
    textview.h
    textview.cpp
)

add_library(${LIB_NAME} SHARED ${SRCS} ${QRCS})

target_include_directories(${LIB_NAME} PUBLIC
    ${DtkWidget_INCLUDE_DIRS}
    ${DtkGUI_INCLUDE_DIRS}
)

target_link_libraries(${LIB_NAME} PRIVATE
    ${Qt_LIBS}
    ${DtkWidget_LIBRARIES}
    ${DtkGUI_LIBRARIES}
)

# bin
install(TARGETS ${LIB_NAME} DESTINATION ${PLUGIN_PREVIEW_DIR})

# conf
install(FILES text-preview-plugin.conf DESTINATION ${PLUGIN_PREVIEW_DIR})
