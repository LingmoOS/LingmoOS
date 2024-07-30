add_definitions(-DTRANSLATION_DOMAIN=\"lingmo_wallpaper_org.kde.image\")

set(lingmo-apply-wallpaperimage_SRCS
    lingmo-apply-wallpaperimage.cpp
)
add_executable(lingmo-apply-wallpaperimage ${lingmo-apply-wallpaperimage_SRCS})
target_link_libraries(lingmo-apply-wallpaperimage
    Qt::Core
    Qt::DBus
    KF6::I18n
)

add_subdirectory(plugin)
add_subdirectory(wallpaperfileitemactionplugin)

install(TARGETS lingmo-apply-wallpaperimage DESTINATION ${KDE_INSTALL_BINDIR})

install(FILES wallpaper.knsrc wallpaper-mobile.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})

lingmo_install_package(imagepackage org.kde.image wallpapers wallpaper)
lingmo_install_package(slideshowpackage org.kde.slideshow wallpapers wallpaper)

install(DIRECTORY imagepackage/contents/ui DESTINATION ${LINGMO_DATA_INSTALL_DIR}/wallpapers/org.kde.slideshow/contents PATTERN .svn EXCLUDE PATTERN CMakeLists.txt EXCLUDE PATTERN Messages.sh EXCLUDE)
