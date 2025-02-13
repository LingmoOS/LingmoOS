set(SCREEN_SRCS
    wallpaper.cpp
    wallpaper.h
    plugin.cpp
)

find_package(Qt5 REQUIRED COMPONENTS DBus)

add_library(lingmosystem_qmlplugins SHARED ${SCREEN_SRCS})

target_link_libraries (lingmosystem_qmlplugins
    Qt5::Core
    Qt5::Quick
    Qt5::Gui
    Qt5::DBus
)

install(TARGETS lingmosystem_qmlplugins DESTINATION ${INSTALL_QMLDIR}/Lingmo/System)
install(FILES qmldir DESTINATION ${INSTALL_QMLDIR}/Lingmo/System)