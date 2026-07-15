set(kcm_SRCS
        KCM.h
        KCM.cpp
        plugin.cpp
)

kconfig_add_kcfg_files(kcm_SRCS Config.kcfgc)
qt6_wrap_ui(kcm_SRCS KCM.ui)
qt6_add_dbus_interface(kcm_SRCS ${KWIN_EFFECTS_INTERFACE} kwineffects_interface)

kcoreaddons_add_plugin(kwin_shapecorners_config INSTALL_NAMESPACE "kwin/effects/configs" SOURCES ${kcm_SRCS})

target_link_libraries(kwin_shapecorners_config
    Qt6::DBus
    KF6::ConfigGui
    KF6::ConfigWidgets
    KF6::KCMUtils
)
