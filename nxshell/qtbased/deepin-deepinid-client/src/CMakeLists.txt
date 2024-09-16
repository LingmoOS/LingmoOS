set(CMAKE_CXX_STANDARD 14)

# http://doc.qt.io/qt-5/cmake-manual.html Find includes in corresponding build
# directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)

# Install settings
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

include(GNUInstallDirs)

add_definitions("-DQT_MESSAGELOGCONTEXT")
add_compile_definitions(CVERSION="${CVERSION}")

# Qt Config
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core DBus Gui WebChannel WebEngineCore WebEngineWidgets Widgets LinguistTools)
if (${ENABLE_QT6})
  find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core5Compat)
endif()
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Widget)
# DtkConfig
# dbus_Frame

set(CLIENT_FILES
        ui/login_window.h
        ui/login_window.cpp
        ui/sync_client.h
        ui/sync_client.cpp
        ui/login_page.h
        ui/login_page.cpp
        ui/login_view.h
        ui/login_view.cpp
        utils/utils.cpp
        utils/utils.h
        utils/hardwareinfo.h
        utils/hardwareinfo.cpp
        ipc/const.h
        ipc/deepinid_interface.h
        ipc/deepinid_interface.cpp
        ipc/dbuslogin1manager.h
        ipc/dbuslogin1manager.cpp
        ipc/dbusvariant.h
        ipc/dbusvariant.cpp
        service/session.h
        service/session.cpp
        service/authentication_manager.cpp
        service/authentication_manager.h
        service/authorize.h
        main.cpp)

add_executable(deepin-deepinid-client ${CLIENT_FILES} resource/resource.qrc)

# Executable files Generate .qm files from .ts files.
file(GLOB DMAN_TRANSLATION_TS ${CMAKE_SOURCE_DIR}/translations/*.ts)
set_source_files_properties(${DMAN_TRANSLATION_TS}
        PROPERTIES
        OUTPUT_LOCATION
        ${CMAKE_BINARY_DIR}/translations)

if (${ENABLE_QT6})  
  qt_add_translation(DMAN_TRANSLATION_QM ${DMAN_TRANSLATION_TS})
else() 
  qt5_add_translation(DMAN_TRANSLATION_QM ${DMAN_TRANSLATION_TS})
endif()

add_custom_target(update-qm DEPENDS ${DMAN_TRANSLATION_QM})


set(UOSID_LOGO ${CMAKE_CURRENT_SOURCE_DIR}/resource/icons/deepin-id.svg)
message(STATUS "install logo path:${UOSID_LOGO}")
# 安装图标文件
# install(FILES ${UOSID_LOGO} DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/misc/icons DESTINATION ${CMAKE_INSTALL_DATADIR})

install(TARGETS deepin-deepinid-client DESTINATION ${CMAKE_INSTALL_BINDIR})

install(DIRECTORY ${CMAKE_BINARY_DIR}/translations
        DESTINATION ${CMAKE_INSTALL_DATADIR}/deepin-deepinid-client
        FILES_MATCHING
        PATTERN "*")

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/com.deepin.deepinid.Client.service
        DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services/)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resource/privacy
        DESTINATION ${CMAKE_INSTALL_DATADIR}/deepin-deepinid-client
        FILES_MATCHING
        PATTERN "*")

set(COMMON_LIBS
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::WebChannel
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::WebEngineWidgets
    ${DtkWidget_LIBRARIES}
)
if (${ENABLE_QT6})
  list(APPEND COMMON_LIBS Qt6::Core5Compat)
endif()
target_link_libraries(deepin-deepinid-client ${COMMON_LIBS})

add_dependencies(deepin-deepinid-client update-qm)

add_subdirectory(data)
