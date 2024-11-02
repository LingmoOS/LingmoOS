cmake_minimum_required(VERSION 3.14)
project(lingmo-search-app-data-service VERSION 1.0.0 LANGUAGES CXX)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_MICRO 0)
set(LINGMO_SEARCH_APP_DATA_SERVICE_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO})
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core DBus Gui Widgets Sql Network REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core DBus Gui Widgets Sql Network REQUIRED)
find_package(KF5WindowSystem)
find_package(qt5xdg)

set(LINGMO_SEARCH_APP_DATA_SERVICE_SRC
        app-db-manager.cpp app-db-manager.h
        main.cpp
        pending-app-info.h
        pending-app-info-queue.cpp pending-app-info-queue.h
        signal-transformer.cpp signal-transformer.h
        lingmo-search-app-data-service.cpp lingmo-search-app-data-service.h)

if(COMMAND qt_add_dbus_adaptor)
    qt_add_dbus_adaptor(LINGMO_SEARCH_APP_DATA_SERVICE_SRC dbus/org.lingmo.AppDatabase.xml app-db-manager.h LingmoUISearch::AppDBManager)
else()
    qt5_add_dbus_adaptor(LINGMO_SEARCH_APP_DATA_SERVICE_SRC dbus/org.lingmo.AppDatabase.xml app-db-manager.h LingmoUISearch::AppDBManager)
endif()

add_executable(lingmo-search-app-data-service
        ${LINGMO_SEARCH_APP_DATA_SERVICE_SRC}
)
target_include_directories(lingmo-search-app-data-service PRIVATE
    ../3rd-parties/qtsingleapplication/src
    ../libchinese-segmentation
    ../libsearch
    ../libsearch/appdata
    ../libsearch/filesystemwatcher
)

target_compile_definitions(lingmo-search-app-data-service PRIVATE
    QT_DEPRECATED_WARNINGS
    VERSION="${LINGMO_SEARCH_APP_DATA_SERVICE_VERSION}"
)

target_link_libraries(lingmo-search-app-data-service PRIVATE
        KF5::WindowSystem
        Qt5Xdg
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::Sql
        chinese-segmentation
        lingmo-search
        qtsingleapplication
        )

install(TARGETS lingmo-search-app-data-service DESTINATION /usr/bin)
install(FILES ../data/lingmo-search-app-data-service.desktop DESTINATION /etc/xdg/autostart)
install(FILES conf/com.lingmo.search.appdb.service DESTINATION /usr/share/dbus-1/services/)
install(FILES conf/application-dirs.conf DESTINATION /etc/lingmo/lingmo-search/)