# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

#设置包含目录
include_directories( ${PROJECT_SOURCE_DIR}/src/include/ui
                     ${PROJECT_SOURCE_DIR}/src/include/aria2
                     ${PROJECT_SOURCE_DIR}/src/include/log
                     ${PROJECT_SOURCE_DIR}/src/include/ui/mainFrame
                     ${PROJECT_SOURCE_DIR}/src/include/ui/createTask
                     ${PROJECT_SOURCE_DIR}/src/include/ui/settings
                     ${PROJECT_SOURCE_DIR}/src/include/ui/websocket
                     ${CMAKE_BINARY_DIR}
)

link_directories(${CMAKE_BINARY_DIR}/lib)

#预定义宏，主要用于dll导出
ADD_DEFINITIONS(-DLM_CONTROLUI_LIB )

#需要moc(加Q_OBJECT)的头文件 
set (UDLC_UI_HEADERS
    ${PROJECT_SOURCE_DIR}/src/include/ui/mainFrame/mainframe.h
    ${PROJECT_SOURCE_DIR}/src/include/ui/mainFrame/tableView.h
    ${PROJECT_SOURCE_DIR}/src/include/ui/mainFrame/tableModel.h
    ${PROJECT_SOURCE_DIR}/src/include/ui/mainFrame/headerView.h
    ${PROJECT_SOURCE_DIR}/src/include/ui/mainFrame/itemDelegate.h
    ${PROJECT_SOURCE_DIR}/src/include/ui/mainFrame/topButton.h
    ${PROJECT_SOURCE_DIR}/src/include/ui/mainFrame/deleteitemthread.h
    ${PROJECT_SOURCE_DIR}/src/include/ui/mainFrame/tabledatacontrol.h
    ${PROJECT_SOURCE_DIR}/src/include/ui/mainFrame/searchresoultwidget.h

    ${PROJECT_SOURCE_DIR}/src/include/ui/createTask/createtaskwidget.h
    ${PROJECT_SOURCE_DIR}/src/include/ui/createTask/btinfodelegate.h
    ${PROJECT_SOURCE_DIR}/src/include/ui/createTask/btinfodialog.h
    ${PROJECT_SOURCE_DIR}/src/include/ui/createTask/btinfotableview.h
    ${PROJECT_SOURCE_DIR}/src/include/ui/createTask/urlthread.h
    ${PROJECT_SOURCE_DIR}/src/include/ui/createTask/analysisurl.h
    ${PROJECT_SOURCE_DIR}/src/include/ui/createTask/taskdelegate.h
    ${PROJECT_SOURCE_DIR}/src/include/ui/createTask/btheaderview.h
    ${PROJECT_SOURCE_DIR}/src/include/ui/createTask/taskModel.h

    ${PROJECT_SOURCE_DIR}/src/include/ui/settings/notificationssettiingwidget.h
    ${PROJECT_SOURCE_DIR}/src/include/ui/settings/settingswidget.h
    ${PROJECT_SOURCE_DIR}/src/include/ui/settings/downloadsettingwidget.h
    ${PROJECT_SOURCE_DIR}/src/include/ui/settings/filesavepathchooser.h
    ${PROJECT_SOURCE_DIR}/src/include/ui/settings/itemselectionwidget.h
    ${PROJECT_SOURCE_DIR}/src/include/ui/settings/settinginfoinputwidget.h
    ${PROJECT_SOURCE_DIR}/src/include/ui/settings/settings.h
    ${PROJECT_SOURCE_DIR}/src/include/ui/settings/settingslabel.h
    ${PROJECT_SOURCE_DIR}/src/include/ui/settings/diagnostictool.h
    ${PROJECT_SOURCE_DIR}/src/include/ui/settings/httpadvancedsettingwidget.h
    ${PROJECT_SOURCE_DIR}/src/include/ui/settings/timeEdit.h

    ${PROJECT_SOURCE_DIR}/src/include/ui/global.h
    ${PROJECT_SOURCE_DIR}/src/include/ui/messagebox.h
    ${PROJECT_SOURCE_DIR}/src/include/ui/clipboardtimer.h
    ${PROJECT_SOURCE_DIR}/src/include/ui/dlmapplication.h
    ${PROJECT_SOURCE_DIR}/src/include/ui/func.h
)



#发布的时候生成qm文件
#if (NOT (${CMAKE_BUILD_TYPE} MATCHES "Debug"))
#    #生成 ts qm文件
#    set(TS_FILES  ${PROJECT_SOURCE_DIR}  "${PROJECT_SOURCE_DIR}/translations/downloader_zh_CN.ts")
#    qt5_create_translation(QM_FILES
#        ${PROJECT_SOURCE_DIR}/translations
#        ${TS_FILES}
#        OPTIONS -source-language en_US -no-obsolete)

#    set(TS_FILES  ${PROJECT_SOURCE_DIR}  "${PROJECT_SOURCE_DIR}/translations/downloader_en_US.ts")
#    qt5_create_translation(QM_FILES
#        ${PROJECT_SOURCE_DIR}/translations
#        ${TS_FILES}
#        OPTIONS -source-language en_US -no-obsolete)
#endif ()


#需要生成的moc文件，输出文件名称放在变量 mocfiles中，必须在find QT5 package才能调用
QT5_WRAP_CPP (UDLC_UI_HEADERS_MOC ${UDLC_UI_HEADERS})

#添加资源文件
set(QRC_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/resource/resource.qrc)
qt5_add_resources(QRC_FILES ${QRC_SOURCE_FILES})

#设置输出dll的名称、类型(动态库、静态库)及所用到的文件
add_library(dlmcontrolui SHARED
       mainFrame/mainframe.cpp
       mainFrame/tableView.cpp
       mainFrame/tableModel.cpp
       mainFrame/tabledatacontrol.cpp
       mainFrame/headerView.cpp
       mainFrame/itemDelegate.cpp
       mainFrame/topButton.cpp
       mainFrame/deleteitemthread.cpp
       mainFrame/searchresoultwidget.cpp
       createTask/createtaskwidget.cpp
       createTask/btinfodelegate.cpp
       createTask/btinfodialog.cpp
       createTask/btinfotableview.cpp
       createTask/btheaderview.cpp
       createTask/urlthread.cpp
       createTask/analysisurl.cpp
       createTask/taskdelegate.cpp
       createTask/taskModel.cpp
       settings/notificationssettiingwidget.cpp
       settings/settingswidget.cpp
       settings/downloadsettingwidget.cpp
       settings/filesavepathchooser.cpp
       settings/itemselectionwidget.cpp
       settings/settinginfoinputwidget.cpp
       settings/settings.cpp
       settings/settingslabel.cpp
       settings/diagnostictool.cpp
       settings/httpadvancedsettingwidget.cpp
       settings/timeEdit.cpp
       clipboardtimer.cpp
       messagebox.cpp
       dlmapplication.cpp
       func.cpp
       ${UDLC_UI_HEADERS_MOC}
       ${QRC_FILES}
       ${QM_FILES}
)

target_link_libraries(dlmcontrolui 
    dlmlog
    dlmaria2
    dlmdatabase
    ${DtkCore_LIBRARIES}
    ${DtkWidget_LIBRARIES}
    ${DtkGui_LIBRARIES}
    Qt5::Core
    Qt5::Network
    Qt5::Widgets
    Qt5::Svg
    Qt5::DBus
)

#install(TARGETS dlmcontrolui DESTINATION lib)

