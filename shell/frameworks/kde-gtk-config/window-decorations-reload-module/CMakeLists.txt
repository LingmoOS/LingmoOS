add_library(window-decorations-gtk3-module MODULE
    reloader.c
)

target_link_libraries(window-decorations-gtk3-module
  PRIVATE
    PkgConfig::GLib2
    PkgConfig::GTK+3
)

set_target_properties(window-decorations-gtk3-module
  PROPERTIES
    OUTPUT_NAME libwindow-decorations-gtk-module
)

install(
    TARGETS window-decorations-gtk3-module
    DESTINATION ${KDE_INSTALL_LIBDIR}/gtk-3.0/modules
)

