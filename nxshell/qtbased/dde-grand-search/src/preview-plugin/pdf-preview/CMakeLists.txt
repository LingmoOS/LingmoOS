project(pdf-preview)

# 定义插件名称
set(LIB_NAME pdf-preview-plugin)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PDF REQUIRED deepin-pdfium)
pkg_check_modules(JPEG REQUIRED libjpeg)

set(SRCS
    pdfpreview_global.h
    pdfpreviewinterface.h
    pdfpreviewinterface.cpp
    pdfpreviewplugin.h
    pdfpreviewplugin.cpp
    pdfview.h
    pdfview.cpp
)

add_library(${LIB_NAME} SHARED ${SRCS} ${QRCS})

target_include_directories(${LIB_NAME} PUBLIC
    ${DtkWidget_INCLUDE_DIRS}
    ${DtkGUI_INCLUDE_DIRS}
    ${PDF_INCLUDE_DIRS}
)

target_link_libraries(${LIB_NAME} PRIVATE
    ${Qt_LIBS}
    ${DtkWidget_LIBRARIES}
    ${DtkGUI_LIBRARIES}
    ${PDF_LIBRARIES}
    ${JPEG_LIBRARIES}
)

# bin
install(TARGETS ${LIB_NAME} DESTINATION ${PLUGIN_PREVIEW_DIR})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/pdf-preview-plugin.conf DESTINATION ${PLUGIN_PREVIEW_DIR})
