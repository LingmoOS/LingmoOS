# The launcher without its view: its applications in the user's order, the
# search, the pages, and org.lingmo.Launcher.
add_library(lingmo-shell-launcher STATIC
    launcher.cpp
    launcher.h
    launcher_model.cpp
    launcher_model.h
)
target_link_libraries(lingmo-shell-launcher PUBLIC lingmo-shell-dock)
