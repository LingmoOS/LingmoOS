add_subdirectory(src)
if(BUILD_TESTING)
    add_subdirectory(autotests)
endif()

install(FILES org.kde.iconexplorer.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.plasma.iconexplorer.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
