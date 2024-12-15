project(lingmo-settings-daemon)
set(TARGET lingmo-settings-daemon)

find_package(ECM REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(X11 REQUIRED)
find_package(X11_XCB REQUIRED)
find_package(XCB REQUIRED)

pkg_check_modules(XORGLIBINPUT xorg-libinput IMPORTED_TARGET)
pkg_check_modules(XORGSERVER xorg-server IMPORTED_TARGET)
pkg_check_modules(SYNAPTICS xorg-synaptics IMPORTED_TARGET GLOBAL)

file (GLOB_RECURSE SRCS
  "*.cpp"
  "*.h"
  "theme/*.h"
  "theme/*.cpp"
  "brightness/*.h"
  "brightness/*.cpp"
  "brightness/*.c"
  "battery/*.cpp"
  "battery/*.h"
  "language/*.cpp"
  "language/*.h"
  "dock/*.cpp"
  "dock/*.h"
  "mouse/*.h"
  "mouse/*.cpp"
  "touchpad/*.h"
  "touchpad/*.cpp"
  "touchpad/x11/*.h"
  "touchpad/x11/*.cpp"
)

set(SOURCES ${SRCS})
set(HEADERS "")
set(FORMS "")
set(RESOURCES "")

qt5_add_dbus_adaptor(DBUS_SOURCES
                     brightness/com.lingmo.Brightness.xml
                     brightness/brightnessmanager.h BrightnessManager)
qt5_add_dbus_adaptor(DBUS_SOURCES
                     theme/com.lingmo.Theme.xml
                     theme/thememanager.h ThemeManager)
qt5_add_dbus_adaptor(DBUS_SOURCES
                     battery/com.lingmo.PrimaryBattery.xml
                     battery/battery.h Battery)
qt5_add_dbus_adaptor(DBUS_SOURCES
                     language/com.lingmo.Language.xml
                     language/language.h Language)
qt5_add_dbus_adaptor(DBUS_SOURCES
                     dock/com.lingmo.Dock.xml
                     dock/dock.h Dock)
qt5_add_dbus_adaptor(DBUS_SOURCES
                     mouse/com.lingmo.Mouse.xml
                     mouse/mousemanager.h Mouse)
qt5_add_dbus_adaptor(DBUS_SOURCES
                     touchpad/com.lingmo.Touchpad.xml
                     touchpad/touchpadmanager.h TouchpadManager)
set_source_files_properties(${DBUS_SOURCES} PROPERTIES SKIP_AUTOGEN ON)

add_executable(${TARGET} ${SOURCES} ${DBUS_SOURCES} ${HEADERS} ${FORMS} ${RESOURCES})

file(GLOB TS_FILES translations/*.ts)
foreach(filepath ${TS_FILES})
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" filename ${filepath})
    list(APPEND ts_files_replaced ${filename})
endforeach()
qt5_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${ts_files_replaced})

target_link_libraries(${TARGET}
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::DBus
        Qt5::X11Extras
        Qt5::Xml
        ${X11_LIBRARIES}
        X11::X11
        X11::Xi
        X11::XCB
        ${XCB_LIBRARIES}
        PkgConfig::XORGLIBINPUT
        PkgConfig::XORGSERVER
        PkgConfig::SYNAPTICS
)

add_custom_target(translations DEPENDS ${QM_FILES} SOURCES ${ts_files_replaced})
add_dependencies(${TARGET} translations)

install(TARGETS ${TARGET} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${TARGET}/translations)
