add_subdirectory(v2)

if (KWIN_BUILD_KCMS)
    set(deepin-kcm_kwin4_genericscripted_SRCS genericscriptedconfig.cpp)
    if (QT_MAJOR_VERSION EQUAL "5")
        qt5_add_dbus_interface(deepin-kcm_kwin4_genericscripted_SRCS ${kwin_effects_dbus_xml} kwineffects_interface)
    else()
        qt_add_dbus_interface(deepin-kcm_kwin4_genericscripted_SRCS ${kwin_effects_dbus_xml} kwineffects_interface)
    endif()
    add_library(deepin-kcm_kwin4_genericscripted MODULE ${deepin-kcm_kwin4_genericscripted_SRCS})
    target_link_libraries(deepin-kcm_kwin4_genericscripted
        KF5::ConfigWidgets #KCModule
        KF5::I18n
        KF5::Service
        Qt${QT_MAJOR_VERSION}::DBus
        Qt${QT_MAJOR_VERSION}::UiTools
        kwineffects
    )
    install(TARGETS deepin-kcm_kwin4_genericscripted DESTINATION ${KDE_INSTALL_PLUGINDIR}/deepin-kwin/effects/configs)
endif()
