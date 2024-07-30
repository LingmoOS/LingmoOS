add_library(colorreload-gtk3-module MODULE
    reloader.c
)

target_link_libraries(colorreload-gtk3-module
    PRIVATE
      PkgConfig::GLib2 
      PkgConfig::GTK+3 
)

set_target_properties(colorreload-gtk3-module
    PROPERTIES
      OUTPUT_NAME libcolorreload-gtk-module
)

install(
    TARGETS colorreload-gtk3-module 
    DESTINATION ${KDE_INSTALL_LIBDIR}/gtk-3.0/modules
)
