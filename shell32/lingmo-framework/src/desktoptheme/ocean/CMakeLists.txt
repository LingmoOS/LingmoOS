configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/metadata.json.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/metadata.json"
    @ONLY
)

install(
    FILES colors "${CMAKE_CURRENT_BINARY_DIR}/metadata.json"
          plasmarc
    DESTINATION ${PLASMA_DATA_INSTALL_DIR}/desktoptheme/ocean
)

FILE(GLOB widgets widgets/*.svg)
plasma_install_desktoptheme_svgs(ocean SUBPATH widgets FILES ${widgets})

FILE(GLOB dialogs dialogs/*.svgz)
plasma_install_desktoptheme_svgs(ocean SUBPATH dialogs FILES ${dialogs})

FILE(GLOB opaque_dialogs opaque/dialogs/*.svgz)
plasma_install_desktoptheme_svgs(ocean SUBPATH opaque/dialogs FILES ${opaque_dialogs})

FILE(GLOB opaque_widgets opaque/widgets/*.svgz)
plasma_install_desktoptheme_svgs(ocean SUBPATH opaque/widgets FILES ${opaque_widgets})

FILE(GLOB translucent_widgets translucent/widgets/*.svgz)
plasma_install_desktoptheme_svgs(ocean SUBPATH translucent/widgets FILES ${translucent_widgets})

FILE(GLOB translucent_dialogs translucent/dialogs/*.svgz)
plasma_install_desktoptheme_svgs(ocean SUBPATH translucent/dialogs FILES ${translucent_dialogs})

FILE(GLOB icons icons/*.svg)
plasma_install_desktoptheme_svgs(ocean SUBPATH icons FILES ${icons})

FILE(GLOB solid_widgets solid/widgets/*.svgz)
plasma_install_desktoptheme_svgs(ocean SUBPATH solid/widgets FILES ${solid_widgets})

FILE(GLOB solid_dialogs solid/dialogs/*.svgz)
plasma_install_desktoptheme_svgs(ocean SUBPATH solid/dialogs FILES ${solid_dialogs})
