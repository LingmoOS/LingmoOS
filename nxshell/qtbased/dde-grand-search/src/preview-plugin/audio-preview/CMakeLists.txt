project(audio-preview)

# 定义插件名称
set(LIB_NAME audio-preview-plugin)
find_package(PkgConfig REQUIRED)
pkg_check_modules(TAGLIB REQUIRED taglib)
pkg_check_modules(AVFORMAT REQUIRED libavformat)
pkg_check_modules(ICU REQUIRED icu-i18n)

set(SRCS
    audiopreview_global.h
    audiopreviewinterface.h
    audiopreviewinterface.cpp
    audiopreviewplugin.h
    audiopreviewplugin.cpp
    audiofileinfo.h
    audiofileinfo.cpp
    audioview.h
    audioview.cpp
)

add_library(${LIB_NAME} SHARED ${SRCS})

target_include_directories(${LIB_NAME} PUBLIC
    ${DtkWidget_INCLUDE_DIRS}
    ${DtkGUI_INCLUDE_DIRS}
    ${TAGLIB_INCLUDE_DIRS}
    ${AVFORMAT_INCLUDE_DIRS}
)

target_link_libraries(${LIB_NAME} PRIVATE
    ${Qt_LIBS}
    ${DtkWidget_LIBRARIES}
    ${DtkGUI_LIBRARIES}
    ${TAGLIB_LIBRARIES}
    ${AVFORMAT_LIBRARIES}
    ${ICU_LIBRARIES}
)

# bin
install(TARGETS ${LIB_NAME} DESTINATION ${PLUGIN_PREVIEW_DIR})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/audio-preview-plugin.conf DESTINATION ${PLUGIN_PREVIEW_DIR})
