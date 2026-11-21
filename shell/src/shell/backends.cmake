# The shell's backends, one static library per component: models and D-Bus
# services without views, on the toolkit and the framework's plain clients.
find_package(LingmoClients REQUIRED COMPONENTS upower powerprofiles audio networkmanager)

add_subdirectory(src/shell/common)
add_subdirectory(src/shell/notification)
add_subdirectory(src/shell/statusbar)
add_subdirectory(src/shell/dock)
add_subdirectory(src/shell/launcher)
add_subdirectory(src/shell/clipboard)
