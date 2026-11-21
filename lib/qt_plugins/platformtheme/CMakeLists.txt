project(platformthemeplugin)
include(GNUInstallDirs)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets DBus)
find_package(Qt6Gui CONFIG QUIET COMPONENTS Private)
if(NOT TARGET Qt6::GuiPrivate)
    find_package(Qt6 REQUIRED COMPONENTS GuiPrivate)
endif()
find_package(PkgConfig REQUIRED)

set (SRCS
    main.cpp
    waylandexport.h
    waylandexport.cpp
    platformtheme.h
    platformtheme.cpp
    filedialoghelper.h
    filedialoghelper.cpp
    hintsettings.h
    hintsettings.cpp
    touchpadscrolling.h
    touchpadscrolling.cpp
)

# xdg-foreign lets the portal's file dialog parent itself onto the application
# window. Optional: without it dialogs simply open unparented.
pkg_check_modules(WaylandClient QUIET IMPORTED_TARGET wayland-client)
pkg_check_modules(WaylandProtocols QUIET wayland-protocols>=1.32)
if(WaylandClient_FOUND AND WaylandProtocols_FOUND)
    pkg_get_variable(WAYLAND_PROTOCOLS_DATADIR wayland-protocols pkgdatadir)
    pkg_get_variable(WAYLAND_SCANNER wayland-scanner wayland_scanner)
    set(THEME_PROTOCOL_DIR ${CMAKE_CURRENT_BINARY_DIR}/wayland-protocols)
    file(MAKE_DIRECTORY ${THEME_PROTOCOL_DIR})
    set(xdg_foreign_xml
        ${WAYLAND_PROTOCOLS_DATADIR}/unstable/xdg-foreign/xdg-foreign-unstable-v2.xml)
    add_custom_command(OUTPUT ${THEME_PROTOCOL_DIR}/xdg-foreign-unstable-v2-protocol.c
        COMMAND ${WAYLAND_SCANNER} private-code ${xdg_foreign_xml}
            ${THEME_PROTOCOL_DIR}/xdg-foreign-unstable-v2-protocol.c
        DEPENDS ${xdg_foreign_xml} VERBATIM)
    add_custom_command(OUTPUT ${THEME_PROTOCOL_DIR}/xdg-foreign-unstable-v2-client-protocol.h
        COMMAND ${WAYLAND_SCANNER} client-header ${xdg_foreign_xml}
            ${THEME_PROTOCOL_DIR}/xdg-foreign-unstable-v2-client-protocol.h
        DEPENDS ${xdg_foreign_xml} VERBATIM)
    list(APPEND SRCS
        ${THEME_PROTOCOL_DIR}/xdg-foreign-unstable-v2-protocol.c
        ${THEME_PROTOCOL_DIR}/xdg-foreign-unstable-v2-client-protocol.h)
endif()

add_library(lingmoplatformtheme MODULE ${SRCS})

if(WaylandClient_FOUND AND WaylandProtocols_FOUND)
    target_compile_definitions(lingmoplatformtheme PRIVATE LINGMO_PLATFORMTHEME_WAYLAND)
    target_include_directories(lingmoplatformtheme PRIVATE ${THEME_PROTOCOL_DIR})
    target_link_libraries(lingmoplatformtheme PRIVATE PkgConfig::WaylandClient)
endif()

target_compile_definitions(lingmoplatformtheme
    PRIVATE
        "QT_NO_FOREACH"
)

target_link_libraries(lingmoplatformtheme PRIVATE
    Lingmo::Scrolling
    Lingmo::ScrollingWidgets
    Qt6::GuiPrivate
    Qt6::Widgets
    Qt6::Core
    Qt6::DBus
)

find_program(QT_PATHS_EXECUTABLE NAMES qtpaths6 REQUIRED)
execute_process(COMMAND "${QT_PATHS_EXECUTABLE}" -query QT_INSTALL_PLUGINS
    OUTPUT_VARIABLE QT_PLUGINS_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
if(QT_PLUGINS_DIR)
    message(STATUS "Qt6 plugin directory:" "${QT_PLUGINS_DIR}")
else()
    message(FATAL_ERROR "Qt6 plugin directory cannot be detected.")
endif()

install(TARGETS lingmoplatformtheme LIBRARY DESTINATION "${QT_PLUGINS_DIR}/platformthemes")

# Darkly's original Qt Widgets style is vendored under lingmo_style and is
# built by default. Set LINGMO_BUILD_DARKLY_STYLE=OFF only for a reduced build
# that intentionally omits the Widgets style plugin.
option(LINGMO_BUILD_DARKLY_STYLE "Build the vendored Darkly Qt Widgets style" ON)
if(LINGMO_BUILD_DARKLY_STYLE)
    find_package(ECM 5.102 REQUIRED NO_MODULE)
    list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
    # Debian and other distributions install one CMake package per Framework;
    # there is no aggregate KF6Config.cmake to query all components at once.
    find_package(KF6CoreAddons REQUIRED CONFIG)
    find_package(KF6ColorScheme REQUIRED CONFIG)
    find_package(KF6Config REQUIRED CONFIG)
    find_package(KF6GuiAddons REQUIRED CONFIG)
    find_package(KF6I18n REQUIRED CONFIG)
    find_package(KF6IconThemes REQUIRED CONFIG)
    find_package(KF6WindowSystem REQUIRED CONFIG)

    include(GenerateExportHeader)
    include(KDEInstallDirs6)
    include(KDECMakeSettings)
    find_package(Qt6 REQUIRED COMPONENTS Quick QuickWidgets)
    find_package(KF6FrameworkIntegration QUIET CONFIG)
    find_package(KF6KCMUtils QUIET CONFIG)
    find_package(KF6KirigamiPlatform QUIET CONFIG)

        set(LINGMO_STYLE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lingmo_style")
        set(LINGMO_STYLE_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/lingmo_style")
        file(MAKE_DIRECTORY "${LINGMO_STYLE_BINARY_DIR}")
        configure_file("${LINGMO_STYLE_DIR}/config-darkly.h.cmake"
                       "${LINGMO_STYLE_BINARY_DIR}/config-darkly.h")

        set(LINGMO_STYLE_SRCS
            lingmo_style/animations/darklyanimation.cpp
            lingmo_style/animations/darklyanimations.cpp
            lingmo_style/animations/darklyanimationdata.cpp
            lingmo_style/animations/darklybaseengine.cpp
            lingmo_style/animations/darklybusyindicatordata.cpp
            lingmo_style/animations/darklybusyindicatorengine.cpp
            lingmo_style/animations/darklydialdata.cpp
            lingmo_style/animations/darklydialengine.cpp
            lingmo_style/animations/darklyenabledata.cpp
            lingmo_style/animations/darklygenericdata.cpp
            lingmo_style/animations/darklyheaderviewdata.cpp
            lingmo_style/animations/darklyheaderviewengine.cpp
            lingmo_style/animations/darklyscrollbardata.cpp
            lingmo_style/animations/darklyscrollbarengine.cpp
            lingmo_style/animations/darklyspinboxdata.cpp
            lingmo_style/animations/darklyspinboxengine.cpp
            lingmo_style/animations/darklystackedwidgetdata.cpp
            lingmo_style/animations/darklystackedwidgetengine.cpp
            lingmo_style/animations/darklytabbardata.cpp
            lingmo_style/animations/darklytabbarengine.cpp
            lingmo_style/animations/darklytoolboxengine.cpp
            lingmo_style/animations/darklytransitiondata.cpp
            lingmo_style/animations/darklytransitionwidget.cpp
            lingmo_style/animations/darklywidgetstatedata.cpp
            lingmo_style/animations/darklywidgetstateengine.cpp
            lingmo_style/darklyaddeventfilter.cpp
            lingmo_style/darklyblurhelper.cpp
            lingmo_style/darklyboxshadowrenderer.cpp
            lingmo_style/darklyframeshadow.cpp
            lingmo_style/darklyhelper.cpp
            lingmo_style/darklymdiwindowshadow.cpp
            lingmo_style/darklymnemonics.cpp
            lingmo_style/darklypropertynames.cpp
            lingmo_style/darklyshadowhelper.cpp
            lingmo_style/darklysplitterproxy.cpp
            lingmo_style/darklystyle.cpp
            lingmo_style/darklystyleplugin.cpp
            lingmo_style/darklytileset.cpp
            lingmo_style/darklytoolsareamanager.cpp
            lingmo_style/darklywindowmanager.cpp
        )

        kconfig_add_kcfg_files(LINGMO_STYLE_SRCS
            lingmo_style/darklysettings.kcfgc
            lingmo_style/darklystyleconfigdata.kcfgc)
        add_library(darkly6lingmo MODULE ${LINGMO_STYLE_SRCS})
        target_compile_features(darkly6lingmo PRIVATE cxx_std_20)
        set_target_properties(darkly6lingmo PROPERTIES OUTPUT_NAME lingmostyle)
        target_include_directories(darkly6lingmo PRIVATE
            "${LINGMO_STYLE_DIR}"
            "${LINGMO_STYLE_DIR}/animations"
            "${LINGMO_STYLE_BINARY_DIR}")
        target_compile_definitions(darkly6lingmo PRIVATE
            DARKLY_HAVE_KSTYLE=0 DARKLY_HAVE_X11=0 DARKLY_HAVE_KWAYLAND=0)
        target_link_libraries(darkly6lingmo PRIVATE
            Qt6::Core Qt6::Gui Qt6::Widgets Qt6::DBus Qt6::Quick Qt6::QuickWidgets
            KF6::CoreAddons KF6::ColorScheme KF6::ConfigCore KF6::ConfigGui
            KF6::GuiAddons KF6::IconThemes KF6::WindowSystem)
        if(TARGET KF6::Style)
            target_link_libraries(darkly6lingmo PRIVATE KF6::Style)
        endif()
        install(TARGETS darkly6lingmo LIBRARY DESTINATION "${QT_PLUGINS_DIR}/styles")
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
