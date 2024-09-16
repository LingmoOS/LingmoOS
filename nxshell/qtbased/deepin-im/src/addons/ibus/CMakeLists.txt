set(CMAKE_C_STANDARD 99)

set(IBUS_SOURCES
    IBusProxy.h
    IBusProxy.cpp
    ibustypes.h
    ibustypes.cpp
)

qt_add_dbus_interface(IC_SOURCES
    ${CMAKE_SOURCE_DIR}/misc/interfaces/org.freedesktop.IBus.InputContext.xml
    IBUSInputContextIface
)

qt_add_dbus_interface(IC_SOURCES
    ${CMAKE_SOURCE_DIR}/misc/interfaces/org.freedesktop.IBus.Portal.xml
    IBUSPortIface
)

set_source_files_properties(${CMAKE_SOURCE_DIR}/misc/interfaces/org.freedesktop.IBus.xml
    PROPERTIES NO_NAMESPACE ON
)

qt_add_dbus_interface(IC_SOURCES
    ${CMAKE_SOURCE_DIR}/misc/interfaces/org.freedesktop.IBus.xml
    IBUSIface
)

add_library(ibus MODULE
    ${IBUS_SOURCES}
    ${IC_SOURCES}
)

target_compile_definitions(ibus PRIVATE -DQT_NO_KEYWORDS)

target_link_libraries(ibus PRIVATE
    dimcore
    Qt::Core
    Qt::DBus
    Qt::Gui
    Qt::GuiPrivate
    PkgConfig::gio
    PkgConfig::wlroots
    $<$<BOOL:${Dtk6Core_FOUND}>:Dtk6::Core>
    wlc
    wl_addons_base
)

install(TARGETS ibus
    LIBRARY DESTINATION ${DIM_ADDON_DIR}
)

install(FILES ibus.conf
    DESTINATION ${DIM_ADDON_INFO_DIR}
)


