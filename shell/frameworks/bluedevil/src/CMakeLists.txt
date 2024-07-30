add_definitions(-DTRANSLATION_DOMAIN="bluedevil")

add_subdirectory(applet)
add_subdirectory(kcm)
add_subdirectory(kded)
add_subdirectory(kio)
add_subdirectory(sendfile)
add_subdirectory(wizard)

install(FILES bluedevil.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR})

install(FILES bluedevil-mime.xml DESTINATION ${KDE_INSTALL_MIMEDIR})
update_xdg_mimetypes(${KDE_INSTALL_MIMEDIR})
