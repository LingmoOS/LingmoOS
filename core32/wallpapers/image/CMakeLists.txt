add_definitions(-DTRANSLATION_DOMAIN=\"plasma_wallpaper_org.kde.image\")

set(lingmo-apply-wallpaperimage_SRCS
    lingmo-apply-wallpaperimage.cpp
)
add_executable(lingmo-apply-wallpaperimage ${lingmo-apply-wallpaperimage_SRCS})
target_link_libraries(lingmo-apply-wallpaperimage
    Qt::Core
    Qt::DBus
    KF5::I18n
)

add_subdirectory(plugin)

install(TARGETS lingmo-apply-wallpaperimage DESTINATION ${KDE_INSTALL_BINDIR})

install(FILES wallpaper.knsrc wallpaper-mobile.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})

plasma_install_package(imagepackage org.kde.image wallpapers wallpaper)
plasma_install_package(slideshowpackage org.kde.slideshow wallpapers wallpaper)

configure_file(setaswallpaper.desktop.in setaswallpaper.desktop)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/setaswallpaper.desktop DESTINATION ${KDE_INSTALL_DATADIR}/kio/servicemenus)

install(DIRECTORY imagepackage/contents/ui DESTINATION ${PLASMA_DATA_INSTALL_DIR}/wallpapers/org.kde.slideshow/contents PATTERN .svn EXCLUDE PATTERN CMakeLists.txt EXCLUDE PATTERN Messages.sh EXCLUDE)
