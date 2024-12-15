project(video-preview)

# 定义插件名称
set(LIB_NAME video-preview-plugin)

pkg_check_modules(Avformat REQUIRED libavformat)
pkg_check_modules(Ffm REQUIRED libffmpegthumbnailer)

set(SRCS
    videopreview_global.h
    videopreviewinterface.h
    videopreviewinterface.cpp
    videopreviewplugin.h
    videopreviewplugin.cpp
    videoview.h
    videoview.cpp
)

add_library(${LIB_NAME} SHARED ${SRCS} ${QRCS})

target_include_directories(${LIB_NAME} PUBLIC
    ${DtkWidget_INCLUDE_DIRS}
    ${DtkGUI_INCLUDE_DIRS}
    ${Avformat_INCLUDE_DIRS}
    ${Ffm_INCLUDE_DIRS}
)

target_link_libraries(${LIB_NAME} PRIVATE
    ${Qt_LIBS}
    ${DtkWidget_LIBRARIES}
    ${DtkGUI_LIBRARIES}
    ${Avformat_LIBRARIES}
    ${Ffm_LIBRARIES}
)

# bin
install(TARGETS ${LIB_NAME} DESTINATION ${PLUGIN_PREVIEW_DIR})

# conf
install(FILES video-preview-plugin.conf DESTINATION ${PLUGIN_PREVIEW_DIR})
