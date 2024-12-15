add_subdirectory(v2)

if (KWIN_BUILD_KCMS)
    set(lingmo-kcm_kwin4_genericscripted_SRCS genericscriptedconfig.cpp)
    if (QT_MAJOR_VERSION EQUAL "5")
        qt5_add_dbus_interface(lingmo-kcm_kwin4_genericscripted_SRCS ${kwin_effects_dbus_xml} kwineffects_interface)
    else()
        qt_add_dbus_interface(lingmo-kcm_kwin4_genericscripted_SRCS ${kwin_effects_dbus_xml} kwineffects_interface)
    endif()
    add_library(lingmo-kcm_kwin4_genericscripted MODULE ${lingmo-kcm_kwin4_genericscripted_SRCS})
    target_link_libraries(lingmo-kcm_kwin4_genericscripted
        KF5::ConfigWidgets #KCModule
        KF5::I18n
        KF5::Service
        Qt${QT_MAJOR_VERSION}::DBus
        Qt${QT_MAJOR_VERSION}::UiTools
        kwineffects
    )
    install(TARGETS lingmo-kcm_kwin4_genericscripted DESTINATION ${KDE_INSTALL_PLUGINDIR}/lingmo-kwin/effects/configs)
endif()
