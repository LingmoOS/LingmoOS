# The status bar's global menu bar: the user's menu, the active application's
# name and its exported menus, and the desktop's menus.
find_package(LingmoClients REQUIRED COMPONENTS accounts)

add_library(lingmo-shell-menubar STATIC
    src/shell/menubar/menu_bar_model.cpp
    src/shell/menubar/menu_bar_model.h
    src/shell/menubar/menu_bar_view.cpp
    src/shell/menubar/menu_bar_view.h
)
target_link_libraries(lingmo-shell-menubar PUBLIC lingmo-shell lingmo-shell-statusbar
    lingmo-shell-statusbar-view Lingmo::AccountsClient)
target_link_libraries(lingmo-aurora PRIVATE lingmo-shell-menubar)
