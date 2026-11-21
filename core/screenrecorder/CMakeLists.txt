# lingmo-screenrecorder: the shell's screen recordings, captured through the
# compositor and encoded here, so neither the shell nor the compositor ever
# encodes. It captures only on the Wayland connection the shell starts it with
# (WAYLAND_SOCKET), not by its path. Frames come as DMA-BUFs and stay on the
# GPU through VA-API where it can.
pkg_check_modules(WaylandClient REQUIRED IMPORTED_TARGET wayland-client)
pkg_check_modules(WaylandProtocols REQUIRED wayland-protocols>=1.39)
pkg_check_modules(Gbm REQUIRED IMPORTED_TARGET gbm)
pkg_check_modules(FFmpeg REQUIRED IMPORTED_TARGET libavcodec libavformat libavutil libswscale)
pkg_check_modules(LibVA REQUIRED IMPORTED_TARGET libva)
pkg_check_modules(Libdrm REQUIRED IMPORTED_TARGET libdrm)
pkg_check_modules(PipeWire REQUIRED IMPORTED_TARGET libpipewire-0.3)
pkg_get_variable(WAYLAND_PROTOCOLS_DATADIR wayland-protocols pkgdatadir)
find_program(WAYLAND_SCANNER_EXECUTABLE NAMES wayland-scanner REQUIRED)

add_executable(lingmo-screenrecorder
    main.cpp
    audiocapture.cpp
    capture_client.cpp
    capture_session.cpp
    encoder.cpp
    image.cpp
    recorder.cpp
)

set(protocol_dir ${CMAKE_CURRENT_BINARY_DIR}/protocols)
file(MAKE_DIRECTORY ${protocol_dir})
foreach(protocol
        staging/ext-image-capture-source/ext-image-capture-source-v1
        staging/ext-image-copy-capture/ext-image-copy-capture-v1
        staging/ext-foreign-toplevel-list/ext-foreign-toplevel-list-v1
        stable/linux-dmabuf/linux-dmabuf-v1
        unstable/xdg-output/xdg-output-unstable-v1)
    get_filename_component(name ${protocol} NAME)
    set(xml ${WAYLAND_PROTOCOLS_DATADIR}/${protocol}.xml)
    set(header ${protocol_dir}/${name}-client-protocol.h)
    set(code ${protocol_dir}/${name}-protocol.c)
    add_custom_command(OUTPUT ${header}
        COMMAND ${WAYLAND_SCANNER_EXECUTABLE} client-header ${xml} ${header}
        DEPENDS ${xml} VERBATIM)
    add_custom_command(OUTPUT ${code}
        COMMAND ${WAYLAND_SCANNER_EXECUTABLE} private-code ${xml} ${code}
        DEPENDS ${xml} VERBATIM)
    target_sources(lingmo-screenrecorder PRIVATE ${header} ${code})
endforeach()

target_include_directories(lingmo-screenrecorder PRIVATE ${protocol_dir})
target_link_libraries(lingmo-screenrecorder PRIVATE
    Lingmo::SDKBase
    PkgConfig::WaylandClient
    PkgConfig::Gbm
    PkgConfig::FFmpeg
    PkgConfig::LibVA
    PkgConfig::Libdrm
    PkgConfig::PipeWire
)

install(TARGETS lingmo-screenrecorder
    DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}
    COMPONENT Runtime
)
