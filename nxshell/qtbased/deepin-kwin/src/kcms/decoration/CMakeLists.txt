# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"deepin-kcm_kwindecoration\")

add_subdirectory(declarative-plugin)

set(kcmkwindecoration_SRCS
    declarative-plugin/buttonsmodel.cpp
    decorationmodel.cpp
    kcm.cpp
    utils.cpp
)

kcmutils_generate_module_data(
    kcmkwindecoration_SRCS
    MODULE_DATA_HEADER kwindecorationdata.h
    MODULE_DATA_CLASS_NAME KWinDecorationData
    SETTINGS_HEADERS deepin-kwindecorationsettings.h
    SETTINGS_CLASSES KWinDecorationSettings
)

kconfig_add_kcfg_files(kcmkwindecoration_SRCS deepin-kwindecorationsettings.kcfgc GENERATE_MOC)

kcoreaddons_add_plugin(deepin-kcm_kwindecoration SOURCES ${kcmkwindecoration_SRCS} INSTALL_NAMESPACE "plasma/kcms/systemsettings")

target_link_libraries(deepin-kcm_kwindecoration
    KDecoration2::KDecoration
    KF5::I18n
    KF5::KCMUtils
    KF5::NewStuff
    KF5::QuickAddons
    Qt${QT_MAJOR_VERSION}::Quick
)

set(kwin-applywindowdecoration_SRCS
    kwin-applywindowdecoration.cpp
    decorationmodel.cpp
    utils.cpp
)

kconfig_add_kcfg_files(kwin-applywindowdecoration_SRCS deepin-kwindecorationsettings.kcfgc GENERATE_MOC)
add_executable(kwin-applywindowdecoration ${kwin-applywindowdecoration_SRCS})

set_target_properties(kwin-applywindowdecoration PROPERTIES
    OUTPUT_NAME deepin-kwin-applywindowdecoration
)

target_link_libraries(kwin-applywindowdecoration
    KDecoration2::KDecoration
    Qt${QT_MAJOR_VERSION}::DBus
    KF5::I18n
    KF5::KCMUtils
)

configure_file(deepin-window-decorations.knsrc.cmake ${CMAKE_CURRENT_BINARY_DIR}/deepin-window-decorations.knsrc)

# This desktop file is installed only for retrocompatibility with sycoca
install(FILES deepin-kwindecorationsettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/deepin-window-decorations.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})
install(TARGETS kwin-applywindowdecoration DESTINATION ${KDE_INSTALL_LIBEXECDIR})

kpackage_install_package(package deepin-kcm_kwindecoration kcms)
