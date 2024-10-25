function(install_package_structure name)
    kcoreaddons_add_plugin(${name} SOURCES ${name}_packagestructure.cpp INSTALL_NAMESPACE "kf6/packagestructure")
    target_link_libraries(${name} lingmo_packagestructure_static)
endfunction()

function(install_package_structure_source name source)
    kcoreaddons_add_plugin(${name} SOURCES ${source} INSTALL_NAMESPACE "kf6/packagestructure")
    target_link_libraries(${name} KF6::Package)
endfunction()

add_library(lingmo_packagestructure_static STATIC packages.cpp)
target_link_libraries(lingmo_packagestructure_static KF6::Package Lingmo::Lingmo)
set_property(TARGET lingmo_packagestructure_static PROPERTY POSITION_INDEPENDENT_CODE ON)

install_package_structure(lingmo_generic)
install_package_structure(lingmo_applet)
install_package_structure(lingmo_theme)
install_package_structure(lingmo_containmentactions)

install_package_structure_source(lingmo_wallpaper qmlWallpaper/wallpaper.cpp)
install_package_structure_source(lingmo_shell shell/shellpackage.cpp)
