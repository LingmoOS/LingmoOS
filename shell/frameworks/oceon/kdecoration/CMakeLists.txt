add_definitions(-DTRANSLATION_DOMAIN="ocean_kwin_deco")

################# includes #################
include_directories(${CMAKE_SOURCE_DIR}/liboceancommon)
include_directories(${CMAKE_BINARY_DIR}/liboceancommon)

################# newt target #################
### plugin classes
set(oceandecoration_SRCS
    oceanbutton.cpp
    oceandecoration.cpp
    oceansettingsprovider.cpp
    oceanexceptionlist.cpp
)

### build library
kconfig_add_kcfg_files(oceandecoration_SRCS oceansettings.kcfgc)
add_library(oceandecoration MODULE
    ${oceandecoration_SRCS}
    ${oceandecoration_config_PART_FORMS_HEADERS})

set_target_properties(oceandecoration PROPERTIES
    OUTPUT_NAME org.kde.ocean
)

target_link_libraries(oceandecoration
    PRIVATE
        oceancommon6
        Qt6::DBus
        KF6::CoreAddons
        KF6::ConfigGui
        KF6::GuiAddons
        KF6::I18n
        KF6::IconThemes
        KDecoration2::KDecoration
)

install(TARGETS oceandecoration DESTINATION ${KDE_INSTALL_PLUGINDIR}/${KDECORATION_PLUGIN_DIR})

add_subdirectory(config)

