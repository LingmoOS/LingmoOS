find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Qml Quick)
find_package(Dtk${DTK_VERSION_MAJOR}Core REQUIRED)
find_package(Dtk${DTK_VERSION_MAJOR}Gui REQUIRED)

add_library(${PLUGIN_NAME}_interface INTERFACE)
target_sources(${PLUGIN_NAME}_interface INTERFACE
    ${PROJECT_SOURCE_DIR}/qmlplugin/qmlplugin_plugin.cpp
    ${PROJECT_SOURCE_DIR}/qmlplugin/qmlplugin_plugin.h
)

target_link_libraries(${PLUGIN_NAME}_interface INTERFACE
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::QmlPrivate
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::QuickPrivate
    Dtk${DTK_VERSION_MAJOR}::Core
    Dtk${DTK_VERSION_MAJOR}::Gui
)
