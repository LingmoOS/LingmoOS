################# Qt/KDE #################
find_package(Qt5 REQUIRED CONFIG COMPONENTS Widgets DBus)
find_package(KF5 REQUIRED COMPONENTS
    I18n
    Config
    GuiAddons
    IconThemes
    ConfigWidgets
    WindowSystem)

find_package(Qt5 COMPONENTS Quick)
set(OCEAN_HAVE_QTQUICK ${Qt5Quick_FOUND})

find_package( KF5FrameworkIntegration CONFIG )
set_package_properties(KF5FrameworkIntegration PROPERTIES
    DESCRIPTION "KF5 Framework Integration"
    URL "https://projects.kde.org/projects/frameworks/frameworkintegration"
    TYPE OPTIONAL
    PURPOSE "Required to use KStyle convenience functionalities in style")

set(OCEAN_HAVE_KSTYLE ${KF5FrameworkIntegration_FOUND})

find_package(XCB COMPONENTS XCB)
set_package_properties(XCB PROPERTIES
    DESCRIPTION "X protocol C-language Binding"
    URL "https://xcb.freedesktop.org"
    TYPE OPTIONAL
    PURPOSE "Required to pass style properties to native Windows on X11 Platform"
)

find_package(KF5Wayland CONFIG)
set(OCEAN_HAVE_KWAYLAND ${KF5Wayland_FOUND})

if(UNIX AND NOT APPLE)
    set(OCEAN_HAVE_X11 ${XCB_XCB_FOUND})
    if (XCB_XCB_FOUND)
        find_package(Qt5 REQUIRED CONFIG COMPONENTS X11Extras)
    endif()
else()
    set(OCEAN_HAVE_X11 FALSE)
endif()


################# includes #################
include_directories(
  animations
  debug
)

include_directories(${CMAKE_SOURCE_DIR}/liboceancommon)
include_directories(${CMAKE_BINARY_DIR}/liboceancommon)

################# configuration #################
configure_file(config-ocean.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ocean.h )

########### next target ###############
set(ocean_PART_SRCS
    animations/oceananimation.cpp
    animations/oceananimations.cpp
    animations/oceananimationdata.cpp
    animations/oceanbaseengine.cpp
    animations/oceanbusyindicatordata.cpp
    animations/oceanbusyindicatorengine.cpp
    animations/oceandialdata.cpp
    animations/oceandialengine.cpp
    animations/oceanenabledata.cpp
    animations/oceangenericdata.cpp
    animations/oceanheaderviewdata.cpp
    animations/oceanheaderviewengine.cpp
    animations/oceanscrollbardata.cpp
    animations/oceanscrollbarengine.cpp
    animations/oceanspinboxengine.cpp
    animations/oceanspinboxdata.cpp
    animations/oceanstackedwidgetdata.cpp
    animations/oceanstackedwidgetengine.cpp
    animations/oceantabbarengine.cpp
    animations/oceantabbardata.cpp
    animations/oceantoolboxengine.cpp
    animations/oceantransitiondata.cpp
    animations/oceantransitionwidget.cpp
    animations/oceanwidgetstateengine.cpp
    animations/oceanwidgetstatedata.cpp
    debug/oceanwidgetexplorer.cpp
    oceanaddeventfilter.cpp
    oceanblurhelper.cpp
    oceanframeshadow.cpp
    oceanhelper.cpp
    oceanmdiwindowshadow.cpp
    oceanmnemonics.cpp
    oceanpropertynames.cpp
    oceanshadowhelper.cpp
    oceansplitterproxy.cpp
    oceanstyle.cpp
    oceanstyleplugin.cpp
    oceantileset.cpp
    oceanwindowmanager.cpp
)

kconfig_add_kcfg_files(ocean_PART_SRCS oceanstyleconfigdata.kcfgc)
add_library(ocean MODULE ${ocean_PART_SRCS})
target_link_libraries(ocean Qt5::Core Qt5::Gui Qt5::Widgets Qt5::DBus)
if( OCEAN_HAVE_QTQUICK )
    target_link_libraries(ocean Qt5::Quick)
endif()
target_link_libraries(ocean KF5::ConfigCore KF5::ConfigWidgets KF5::GuiAddons KF5::IconThemes KF5::WindowSystem)
target_link_libraries(ocean oceancommon5)

if(KF5FrameworkIntegration_FOUND)
    target_link_libraries(ocean KF5::Style)
endif()

if (WIN32)
    # As stated in https://docs.microsoft.com/en-us/cpp/c-runtime-library/math-constants M_PI only gets defined
    # when if _USE_MATH_DEFINES is defined
    target_compile_definitions(ocean PRIVATE _USE_MATH_DEFINES _BSD_SOURCE)
endif()

if(OCEAN_HAVE_X11)
    target_link_libraries(ocean ${XCB_LIBRARIES})
    target_link_libraries(ocean Qt5::X11Extras)
endif()

if(OCEAN_HAVE_KWAYLAND)
    target_link_libraries(ocean KF5::WaylandClient)
endif()


########### install files ###############
install(TARGETS ocean DESTINATION ${QT_PLUGIN_INSTALL_DIR}/styles/)
install(FILES ocean.themerc  DESTINATION  ${DATA_INSTALL_DIR}/kstyle/themes)

########### subdirectories ###############
add_subdirectory(config)
