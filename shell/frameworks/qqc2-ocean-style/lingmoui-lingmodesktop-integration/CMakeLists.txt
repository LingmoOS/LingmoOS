
set(org.kde.ocean_SRCS
    lingmodesktoptheme.cpp
    lingmouilingmofactory.cpp
    units.cpp
)

add_library(org.kde.ocean MODULE ${org.kde.ocean_SRCS})

target_link_libraries(org.kde.ocean
    PUBLIC
        Qt::Core
        KF6::LingmoUIPlatform
    PRIVATE
        Qt::Qml
        Qt::Quick
        KF6::GuiAddons
        KF6::ColorScheme
)

if(NOT ANDROID)
    target_link_libraries(org.kde.ocean PRIVATE Qt::DBus KF6::IconThemes)
else()
    set_target_properties(org.kde.ocean PROPERTIES OUTPUT_NAME libplugins_kf6_lingmoui_platform_org.kde.ocean)
endif()

install(TARGETS org.kde.ocean DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/lingmoui/platform)

