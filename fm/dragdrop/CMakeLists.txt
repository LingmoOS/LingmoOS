set(SCREEN_SRCS
    draganddrop/declarativedroparea.cpp
    draganddrop/declarativedragdropevent.cpp
    draganddrop/declarativemimedata.cpp
    plugin.cpp
)

find_package(Qt6 REQUIRED COMPONENTS Core Gui DBus Quick LinguistTools)

add_library(lingmodragdrop_qmlplugins SHARED ${SCREEN_SRCS})

target_link_libraries (lingmodragdrop_qmlplugins
    Qt6::Core
    Qt6::Quick
    Qt6::Gui
    Qt6::DBus
)

install(TARGETS lingmodragdrop_qmlplugins DESTINATION ${INSTALL_QMLDIR}/Lingmo/DragDrop)
install(FILES qmldir DESTINATION ${INSTALL_QMLDIR}/Lingmo/DragDrop)