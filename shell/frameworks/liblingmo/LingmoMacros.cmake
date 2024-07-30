set(LINGMO_RELATIVE_DATA_INSTALL_DIR "lingmo")
set(LINGMO_DATA_INSTALL_DIR "${KDE_INSTALL_DATADIR}/${LINGMO_RELATIVE_DATA_INSTALL_DIR}")

# lingmo_install_package(path componentname [root] [type])
#
# Installs a Lingmo package to the system path
# @arg path The source path to install from, location of metadata.desktop
# @arg componentname The plugin name of the component, corresponding to the
#       X-KDE-PluginInfo-Name key in metadata.desktop
# @arg root The subdirectory to install to, default: plasmoids
# @arg type The type, default to applet, or applet, package, containment,
#       wallpaper, shell, lookandfeel, etc.
# @see Types column in kpackagetool6 --list-types
#
# Examples:
# lingmo_install_package(mywidget org.kde.lingmo.mywidget) # installs an applet
# lingmo_install_package(declarativetoolbox org.kde.toolbox packages package) # installs a generic package
#
macro(lingmo_install_package dir component)
   set(root ${ARGV2})
   set(type ${ARGV3})
   if(NOT root)
      set(root plasmoids)
   endif()
   if(NOT type)
      set(type applet)
   endif()

   kpackage_install_package(${dir} ${component} ${root} ${LINGMO_RELATIVE_DATA_INSTALL_DIR} NO_DEPRECATED_WARNING)
endmacro()
