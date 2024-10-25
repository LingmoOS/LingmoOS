cmake_minimum_required(VERSION 3.5)

project(lingmo-kwin-plugins)

add_subdirectory(plugins)

install(FILES config/kglobalshortcutsrc DESTINATION /etc/xdg)
install(FILES config/kwinrc DESTINATION /etc/xdg)
install(FILES config/kwinrulesrc DESTINATION /etc/xdg)

install(DIRECTORY scripts/linglauncher DESTINATION /usr/share/kwin/scripts)
install(DIRECTORY scripts/ling_squash DESTINATION /usr/share/kwin/effects)
install(DIRECTORY scripts/ling_scale DESTINATION /usr/share/kwin/effects)
install(DIRECTORY scripts/ling_popups DESTINATION /usr/share/kwin/effects)
install(DIRECTORY tabbox/ling_thumbnail DESTINATION /usr/share/kwin/tabbox)
