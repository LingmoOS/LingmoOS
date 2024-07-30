add_definitions(-DTRANSLATION_DOMAIN=\"kcmkgamma\")

add_subdirectory(pics)

kcoreaddons_add_plugin(kcm_kgamma INSTALL_NAMESPACE "plasma/kcms/systemsettings_qwidgets")
kcmutils_generate_desktop_file(kcm_kgamma)

target_sources(kcm_kgamma PRIVATE
   xf86configpath.cpp xf86configpath.h
   xvidextwrap.cpp xvidextwrap.h
   displaynumber.cpp displaynumber.h
   gammactrl.cpp gammactrl.h
   kgamma.cpp kgamma.h
)

target_link_libraries(kcm_kgamma ${X11_Xxf86vm_LIB} ${X11_LIBRARIES} Qt6::Core Qt6::Widgets KF6::ConfigCore KF6::KCMUtils KF6::I18n)

add_custom_command(TARGET kcm_kgamma POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E create_symlink ../kcms/systemsettings_qwidgets/kcm_kgamma.so kcm_kgamma_init.so)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kcm_kgamma_init.so DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/kcminit)

