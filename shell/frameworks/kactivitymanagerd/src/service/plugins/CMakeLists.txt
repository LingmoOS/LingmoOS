# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

function(kamd_add_plugin plugin_name)
   kcoreaddons_add_plugin(${plugin_name} ${ARGN} INSTALL_NAMESPACE ${KAMD_PLUGIN_DIR})
   target_link_libraries(${plugin_name} kactivitymanagerd_plugin)
endfunction()
add_subdirectory (sqlite)
add_subdirectory (activitytemplates)
add_subdirectory (virtualdesktopswitch)
add_subdirectory (globalshortcuts)
add_subdirectory (recentlyused-eventspy)
add_subdirectory (runapplication)
add_subdirectory (krunner)
add_subdirectory (libreoffice-spy)
