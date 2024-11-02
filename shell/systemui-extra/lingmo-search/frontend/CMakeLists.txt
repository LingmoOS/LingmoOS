cmake_minimum_required(VERSION 3.14)
project(frontend VERSION 1.0 LANGUAGES C CXX)

set(VERSION_MAJOR 2)
set(VERSION_MINOR 2)
set(VERSION_MICRO 3)
set(LINGMO_SEARCH_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO})
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core DBus Widgets Xml Concurrent Sql LinguistTools X11Extras REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core DBus Widgets Xml Concurrent Sql LinguistTools X11Extras REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(KF5WindowSystem)
find_package(lingmo-quick COMPONENTS platform REQUIRED)

set(LINGMO_SEARCH_EXTERNAL_LIBS "")
set(LINGMO_SEARCH_PC_PKGS gio-2.0 glib-2.0 gio-unix-2.0 lingmosdk-waylandhelper xapian-core gsettings-qt)

foreach(PC_LIB IN ITEMS ${LINGMO_SEARCH_PC_PKGS})
    pkg_check_modules(${PC_LIB} REQUIRED IMPORTED_TARGET ${PC_LIB})
    if(${${PC_LIB}_FOUND})
        include_directories(${${PC_LIB}_INCLUDE_DIRS})
        link_directories(${${PC_LIB}_LIBRARY_DIRS})
        list(APPEND LINGMO_NOTIFICATION_EXTERNAL_LIBS PkgConfig::${PC_LIB})
    endif()
endforeach()

add_subdirectory(../3rd-parties/qtsingleapplication ${CMAKE_CURRENT_SOURCE_DIR})

set(LINGMO_SEARCH_SRC
        control/create-index-ask-dialog.cpp control/create-index-ask-dialog.h
        control/list-labels/show-more-label.cpp control/list-labels/show-more-label.h
        control/list-labels/title-label.cpp control/list-labels/title-label.h
        control/search-line-edit.cpp control/search-line-edit.h
        control/stack-pages/search-page-section.cpp control/stack-pages/search-page-section.h
        control/stack-pages/search-result-page.cpp control/stack-pages/search-result-page.h
        main.cpp
        mainwindow.cpp mainwindow.h
        model/best-list-model.cpp model/best-list-model.h
        model/search-result-manager.cpp model/search-result-manager.h
        model/search-result-model.cpp model/search-result-model.h
        model/one-result.cpp model/one-result.h
        search-app-widget-plugin/search.cpp search-app-widget-plugin/search.h
        lingmo-search-dbus-service.cpp lingmo-search-dbus-service.h
        lingmo-search-gui.cpp lingmo-search-gui.h
        view/best-list-view.cpp view/best-list-view.h
        view/result-view.cpp view/result-view.h
        view/result-view-delegate.cpp view/result-view-delegate.h
        )
if(COMMAND qt_add_dbus_adaptor)
    qt_add_dbus_adaptor(LINGMO_SEARCH_SRC org.lingmo.search.service.xml lingmo-search-dbus-service.h LingmoUISearch::LingmoUISearchDbusServices)
    qt_add_dbus_interface(LINGMO_SEARCH_SRC org.lingmo.search.service.xml service_interface)
else()
    qt5_add_dbus_adaptor(LINGMO_SEARCH_SRC org.lingmo.search.service.xml lingmo-search-dbus-service.h LingmoUISearch::LingmoUISearchDbusServices)
    qt5_add_dbus_interface(LINGMO_SEARCH_SRC org.lingmo.search.service.xml service_interface)
endif()

set(QRC_FILES resource.qrc)

file(GLOB LINGMO_SEARCH_TS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../translations/lingmo-search/*.ts)
set_source_files_properties(${LINGMO_SEARCH_TS_FILES} PROPERTIES OUTPUT_LOCATION ${CMAKE_BINARY_DIR}/frontend/.qm)
qt5_create_translation(LINGMO_SEARCH_QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${LINGMO_SEARCH_TS_FILES})

file(GLOB LINGMO_SEARCH_APP_WIDGET_TS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../translations/lingmo-search/appwidget/*.ts)
set_source_files_properties(${LINGMO_SEARCH_APP_WIDGET_TS_FILES} PROPERTIES OUTPUT_LOCATION ${CMAKE_BINARY_DIR}/frontend/.qm)
qt5_create_translation(LINGMO_SEARCH_APP_WIDGET_QM_FILES ${PROJECT_SOURCE_DIR} ${LINGMO_SEARCH_APP_WIDGET_TS_FILES})
add_executable(frontend
        ${LINGMO_SEARCH_SRC}
        ${QRC_FILES}
        ${LINGMO_SEARCH_QM_FILES}
        ${LINGMO_SEARCH_APP_WIDGET_QM_FILES})
set_target_properties(frontend
        PROPERTIES
        OUTPUT_NAME lingmo-search
)
set(LINGMO_SEARCH_QM_INSTALL_PATH /usr/share/lingmo-search/translations/lingmo-search)
target_compile_definitions(frontend PRIVATE
        VERSION="${LINGMO_SEARCH_VERSION}"
        LINGMO_SEARCH_QM_INSTALL_PATH="${LINGMO_SEARCH_QM_INSTALL_PATH}"
        )

target_link_libraries(frontend PRIVATE
        KF5::WindowSystem
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::X11Extras
        gsettings-qt
        lingmo-appwidget-manager
        lingmo-appwidget-provider
        lingmo-search
        chinese-segmentation
        lingmosdk-waylandhelper
        qtsingleapplication
        tcmalloc_minimal
        lingmo-quick::platform
        )

target_include_directories(frontend PRIVATE
        ../3rd-parties/qtsingleapplication/src
        ../libchinese-segmentation
        ../libsearch
        ../libsearch/pluginmanage
        ../libsearch/plugininterface
        ../libsearch/searchinterface
        control
        control/list-labels
        control/stack-pages
        model
        search-app-widget-plugin
        view
        )

set(APP_WIDGET_FILES_PRE
    search-app-widget-plugin/provider
)
#小插件相关
install(FILES ${APP_WIDGET_FILES_PRE}/data/search.conf DESTINATION /usr/share/appwidget/config/)
install(FILES ${APP_WIDGET_FILES_PRE}/org.lingmo.appwidget.provider.search.service DESTINATION /usr/share/dbus-1/services/)
install(FILES
        ${APP_WIDGET_FILES_PRE}/data/search.png
        ${APP_WIDGET_FILES_PRE}/data/lingmo-search.svg
        DESTINATION /usr/share/appwidget/search/)
install(FILES ${APP_WIDGET_FILES_PRE}/data/search.qml DESTINATION /usr/share/appwidget/qml/)
install(FILES ${LINGMO_SEARCH_APP_WIDGET_QM_FILES} DESTINATION /usr/share/appwidget/translations/)
#二进制
install(TARGETS frontend RUNTIME DESTINATION /usr/bin)
#翻译
install(FILES ${LINGMO_SEARCH_QM_FILES} DESTINATION ${LINGMO_SEARCH_QM_INSTALL_PATH})
#desktop文件
install(FILES ../data/lingmo-search-menu.desktop DESTINATION /usr/share/applications)
install(FILES ../data/lingmo-search.desktop DESTINATION /etc/xdg/autostart)
#gsettings
install(FILES ../data/org.lingmo.log4qt.lingmo-search.gschema.xml DESTINATION /usr/share/glib-2.0/schemas/)