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
    oceantoolsareamanager.cpp
)

kconfig_add_kcfg_files(ocean_PART_SRCS ../kdecoration/oceansettings.kcfgc)
kconfig_add_kcfg_files(ocean_PART_SRCS oceanstyleconfigdata.kcfgc)


add_library(ocean${QT_MAJOR_VERSION} MODULE ${ocean_PART_SRCS})

ecm_qt_declare_logging_category(ocean${QT_MAJOR_VERSION}
    HEADER
        ocean_logging.h
    IDENTIFIER
        OCEAN
    CATEGORY_NAME
        ocean${QT_MAJOR_VERSION}
    DEFAULT_SEVERITY
        Warning
)

target_link_libraries(ocean${QT_MAJOR_VERSION}
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Widgets
)

if(HAVE_QTDBUS)
    target_link_libraries(ocean${QT_MAJOR_VERSION}
        Qt${QT_MAJOR_VERSION}::DBus
    )
endif()

if( OCEAN_HAVE_QTQUICK )
    target_link_libraries(ocean${QT_MAJOR_VERSION} 
        Qt${QT_MAJOR_VERSION}::Quick
        KF${QT_MAJOR_VERSION}::CoreAddons
    )
endif()

target_link_libraries(ocean${QT_MAJOR_VERSION}
    KF${QT_MAJOR_VERSION}::CoreAddons
    KF${QT_MAJOR_VERSION}::ConfigCore
    KF${QT_MAJOR_VERSION}::ConfigGui
    KF${QT_MAJOR_VERSION}::GuiAddons
    KF${QT_MAJOR_VERSION}::IconThemes
    KF${QT_MAJOR_VERSION}::WindowSystem
)

if(QT_MAJOR_VERSION STREQUAL "5")
    target_link_libraries(ocean5 KF5::ConfigWidgets)
    if (OCEAN_HAVE_QTQUICK)
        target_link_libraries(ocean5 KF5::LingmoUI2)
    endif()
else()
    target_link_libraries(ocean6 KF6::ColorScheme)
    if (OCEAN_HAVE_QTQUICK)
        target_link_libraries(ocean6 KF6::LingmoUIPlatform)
    endif()
endif()

target_link_libraries(ocean${QT_MAJOR_VERSION} oceancommon${QT_MAJOR_VERSION})

if(KF${QT_MAJOR_VERSION}FrameworkIntegration_FOUND)
    target_link_libraries(ocean${QT_MAJOR_VERSION}  KF${QT_MAJOR_VERSION}::Style)
endif()


if (WIN32)
    # As stated in https://docs.microsoft.com/en-us/cpp/c-runtime-library/math-constants M_PI only gets defined
    # when if _USE_MATH_DEFINES is defined
    target_compile_definitions(ocean${QT_MAJOR_VERSION}  PRIVATE _USE_MATH_DEFINES _BSD_SOURCE)
endif()


########### install files ###############
install(TARGETS ocean${QT_MAJOR_VERSION}  DESTINATION ${KDE_INSTALL_QTPLUGINDIR}/styles/)
install(FILES ocean.themerc  DESTINATION  ${KDE_INSTALL_DATADIR}/kstyle/themes)

########### subdirectories ###############

if (QT_MAJOR_VERSION EQUAL "6" AND TARGET "KF6::KCMUtils")
    add_subdirectory(config)
endif()
