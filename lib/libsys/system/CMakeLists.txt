set(SCREEN_SRCS
    wallpaper.cpp
    wallpaper.h
    plugin.cpp
)

find_package(Qt6 REQUIRED COMPONENTS DBus)

add_library(lingmosystem_qmlplugins SHARED ${SCREEN_SRCS})

target_link_libraries (lingmosystem_qmlplugins
    Qt6::Core
    Qt6::Quick
    Qt6::Gui
    Qt6::DBus
)

install(TARGETS lingmosystem_qmlplugins DESTINATION ${INSTALL_QMLDIR}/Lingmo/System)
install(FILES qmldir DESTINATION ${INSTALL_QMLDIR}/Lingmo/System)