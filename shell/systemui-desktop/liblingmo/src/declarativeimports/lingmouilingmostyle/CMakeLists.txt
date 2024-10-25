
add_library(LingmoUILingmoStyle MODULE ${org.kde.desktop_SRCS})

target_sources(LingmoUILingmoStyle PRIVATE
    plugin.cpp
    units.cpp
    lingmotheme.cpp
)

target_link_libraries(LingmoUILingmoStyle
    PUBLIC
        Qt6::Core
        KF6::LingmoUIPlatform
    PRIVATE
        Qt6::Qml
        Qt6::Quick
        KF6::ConfigWidgets
        KF6::IconThemes
        Lingmo::Lingmo
)

install(TARGETS LingmoUILingmoStyle DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/lingmoui/platform)

install(FILES
    AbstractApplicationHeader.qml
    DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/lingmoui/styles/Lingmo
)
