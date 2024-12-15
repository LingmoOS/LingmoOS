set(PLUGIN_NAME "configlib${QT_MAJOR_VERSION}")

add_library(${PLUGIN_NAME} STATIC
    iso639.cpp
    dbusprovider.cpp
    model.cpp
    addonmodel.cpp
    imconfig.cpp
    logging.cpp
    font.cpp
    layoutmodel.cpp
    layoutprovider.cpp
    xkbrules.cpp
    xkbrules.h
)
set_target_properties(${PLUGIN_NAME} PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE
    AUTOMOC TRUE
    AUTOUIC TRUE
    AUTOUIC_OPTIONS "-tr=fcitx::tr2fcitx;--include=fcitxqti18nhelper.h"
)
target_include_directories(${PLUGIN_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(${PLUGIN_NAME} Qt${QT_MAJOR_VERSION}::Core Qt${QT_MAJOR_VERSION}::Gui
    Dtk${QT_MAJOR_VERSION}::Core Dtk${QT_MAJOR_VERSION}::Widget Fcitx5Qt${QT_MAJOR_VERSION}::DBusAddons Fcitx5::Core Fcitx5::Utils)

