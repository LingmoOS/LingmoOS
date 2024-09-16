add_library(configwidgetslib STATIC
    configwidget.cpp
    addonselector.cpp
    categoryhelper.cpp
    erroroverlay.cpp
    fontbutton.cpp
    keylistwidget.cpp
    layoutselector.cpp
    listoptionwidget.cpp
    optionwidget.cpp
    varianthelper.cpp
    verticalscrollarea.cpp
    impage.cpp
)
set_target_properties(configwidgetslib PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE
    AUTOMOC TRUE
    AUTOUIC TRUE
    AUTOUIC_OPTIONS "-tr=fcitx::tr2fcitx;--include=fcitxqti18nhelper.h"
)
target_include_directories(configwidgetslib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(configwidgetslib Qt5::Core Qt5::Widgets Qt5::X11Extras KF5::WidgetsAddons KF5::ItemViews Fcitx5::Utils Fcitx5Qt5::DBusAddons Fcitx5Qt5::WidgetsAddons configlib layoutlib)
