configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/metadata.json.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/metadata.json"
    @ONLY
)

install(
    FILES colors "${CMAKE_CURRENT_BINARY_DIR}/metadata.json"
          lingmorc
    DESTINATION ${LINGMO_DATA_INSTALL_DIR}/desktoptheme/oxygen
)

FILE(GLOB widgets widgets/*.svg)
lingmo_install_desktoptheme_svgs(oxygen SUBPATH widgets FILES ${widgets})

FILE(GLOB dialogs dialogs/*.svg)
lingmo_install_desktoptheme_svgs(oxygen SUBPATH dialogs FILES ${dialogs})

FILE(GLOB opaque opaque/widgets/*.svg)
lingmo_install_desktoptheme_svgs(oxygen SUBPATH opaque/widgets FILES ${opaque})

FILE(GLOB opaque opaque/dialogs/*.svg)
lingmo_install_desktoptheme_svgs(oxygen SUBPATH opaque/dialogs FILES ${opaque})

