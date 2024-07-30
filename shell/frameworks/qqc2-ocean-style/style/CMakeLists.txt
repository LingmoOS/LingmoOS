add_subdirectory(impl)
add_subdirectory(qtquickcontrols)

if(ANDROID)
    install(FILES OceanLight.colors OceanDark.colors DESTINATION ${KDE_INSTALL_DATADIR}/color-schemes)
endif()
