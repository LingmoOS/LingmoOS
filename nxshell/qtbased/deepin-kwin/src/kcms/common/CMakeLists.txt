# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcmkwincommon\")

include_directories(${KWin_SOURCE_DIR}/src/effects)

set(kcmkwincommon_SRC
    effectsmodel.cpp
)

if (QT_MAJOR_VERSION EQUAL "5")
qt5_add_dbus_interface(kcmkwincommon_SRC
    ${KWin_SOURCE_DIR}/src/org.deepin.kwin.Effects.xml kwin_effects_interface
)
qt5_add_dbus_interface(kcmkwincommon_SRC
    ${KWin_SOURCE_DIR}/src/org.deepin.kwin.Xkb.xml kwin_xkb_interface
)
else()
qt_add_dbus_interface(kcmkwincommon_SRC
    ${KWin_SOURCE_DIR}/src/org.deepin.kwin.Effects.xml kwin_effects_interface
)
qt_add_dbus_interface(kcmkwincommon_SRC
    ${KWin_SOURCE_DIR}/src/org.deepin.kwin.Xkb.xml kwin_xkb_interface
)
endif()

add_library(kcmkwincommon SHARED ${kcmkwincommon_SRC})

target_link_libraries(kcmkwincommon
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::DBus
    KF5::CoreAddons
    KF5::ConfigCore
    KF5::I18n
    KF5::Package
    KF5::KCMUtils
)

set_target_properties(kcmkwincommon PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME deepin-kcmkwincommon
)

install(TARGETS kcmkwincommon ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)
