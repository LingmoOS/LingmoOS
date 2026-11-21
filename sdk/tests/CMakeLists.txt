find_package(GTest REQUIRED)
include(GoogleTest)

# Linked against the base library alone, which proves it needs no display.
add_executable(lingmo-sdk-base-tests
    base/collator_test.cpp
    base/dbus_test.cpp
    base/desktop_entry_test.cpp
    base/event_loop_test.cpp
    base/file_test.cpp
    base/hash_test.cpp
    base/helpers_test.cpp
    base/i18n_test.cpp
    base/io_helpers_test.cpp
    base/geometry_test.cpp
    base/json_test.cpp
    base/locale_test.cpp
    base/logging_test.cpp
    base/mime_test.cpp
    base/process_test.cpp
    base/region_test.cpp
    base/settings_test.cpp
    base/signal_test.cpp
    base/strings_test.cpp
    base/thread_pool_test.cpp
    base/timer_test.cpp
    base/transform_test.cpp
    base/user_directory_test.cpp
)

target_link_libraries(lingmo-sdk-base-tests PRIVATE lingmo-sdk-base GTest::gtest_main)

gtest_discover_tests(lingmo-sdk-base-tests)

add_executable(lingmo-sdk-tests
    main.cpp
    about_dialog_test.cpp
    animation_test.cpp
    bitmap_test.cpp
    canvas_2d_test.cpp
    bubble_shape_test.cpp
    clipboard_test.cpp
    color_profile_test.cpp
    color_test.cpp
    controls_test.cpp
    drop_down_popup_test.cpp
    fast_blur_test.cpp
    file_drop_area_test.cpp
    frame_cost_test.cpp
    header_backdrop_test.cpp
    glyph_run_test.cpp
    image_formats_test.cpp
    image_view_test.cpp
    input_test.cpp
    label_test.cpp
    language_change_test.cpp
    layout_cost_test.cpp
    layout_test.cpp
    line_breaks_test.cpp
    menu_test.cpp
    painter_test.cpp
    pixel_test.cpp
    cell_layout_test.cpp
    round_image_button_test.cpp
    scroll_test.cpp
    shell_protocol_test.cpp
    text_area_test.cpp
    text_cost_test.cpp
    text_field_test.cpp
    text_layout_test.cpp
    text_markup_test.cpp
    text_metrics_test.cpp
    window_test.cpp
    window_tab_bar_test.cpp
    window_hints_test.cpp
    window_restore_test.cpp
    compositing_test.cpp
    test_views.h
)

# The image tests write their files with the codecs' own encoders.
pkg_check_modules(ImageTestEncoders REQUIRED IMPORTED_TARGET libjpeg libpng libwebp libtiff-4 libjxl libheif)
target_link_libraries(lingmo-sdk-tests PRIVATE lingmo-sdk GTest::gtest PkgConfig::ImageTestEncoders)
target_compile_definitions(lingmo-sdk-tests
    PRIVATE LINGMO_TOOLKIT_PROTOCOL_DIR="${PROJECT_SOURCE_DIR}/protocols"
    LINGMO_TOOLKIT_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

gtest_discover_tests(lingmo-sdk-tests)
# Read after the discovered tests, which it needs to exist.
set_property(DIRECTORY APPEND PROPERTY TEST_INCLUDE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/serial_tests.cmake)

# One Application per process: the embedded platform gets its own binary.
add_executable(lingmo-sdk-embedded-tests
    embedded/main.cpp
    embedded/embedded_window_test.cpp
    embedded/embedded_controls_test.cpp
    embedded/embedded_gpu_test.cpp
    embedded/embedded_host_test.cpp
)

target_link_libraries(lingmo-sdk-embedded-tests PRIVATE lingmo-sdk GTest::gtest PkgConfig::Xkbcommon
    PkgConfig::Egl PkgConfig::GlesV2)

gtest_discover_tests(lingmo-sdk-embedded-tests)

# A connection against a minimal compositor in the test, on libwayland-server.
pkg_check_modules(WaylandServer REQUIRED IMPORTED_TARGET wayland-server)
set(TEST_PROTOCOL_DIR ${CMAKE_CURRENT_BINARY_DIR}/wayland-protocols)
file(MAKE_DIRECTORY ${TEST_PROTOCOL_DIR})
set(TEST_PROTOCOL_SOURCES)
foreach(xml
        stable/xdg-shell/xdg-shell.xml
        stable/viewporter/viewporter.xml
        staging/fractional-scale/fractional-scale-v1.xml
        unstable/xdg-output/xdg-output-unstable-v1.xml)
    get_filename_component(name ${xml} NAME_WE)
    set(code ${TEST_PROTOCOL_DIR}/${name}-protocol.c)
    set(header ${TEST_PROTOCOL_DIR}/${name}-server-protocol.h)
    add_custom_command(OUTPUT ${code}
        COMMAND ${WAYLAND_SCANNER} private-code ${WAYLAND_PROTOCOLS_DATADIR}/${xml} ${code}
        DEPENDS ${WAYLAND_PROTOCOLS_DATADIR}/${xml} VERBATIM)
    add_custom_command(OUTPUT ${header}
        COMMAND ${WAYLAND_SCANNER} server-header ${WAYLAND_PROTOCOLS_DATADIR}/${xml} ${header}
        DEPENDS ${WAYLAND_PROTOCOLS_DATADIR}/${xml} VERBATIM)
    list(APPEND TEST_PROTOCOL_SOURCES ${code} ${header})
endforeach()
# The project's own protocols, for a compositor that records their requests.
foreach(name lingmo-window-hints-v1 lingmo-window-placement-v1 lingmo-status-item-v1)
    set(code ${TEST_PROTOCOL_DIR}/${name}-protocol.c)
    set(header ${TEST_PROTOCOL_DIR}/${name}-server-protocol.h)
    set(xml ${PROJECT_SOURCE_DIR}/protocols/${name}.xml)
    add_custom_command(OUTPUT ${code}
        COMMAND ${WAYLAND_SCANNER} private-code ${xml} ${code}
        DEPENDS ${xml} VERBATIM)
    add_custom_command(OUTPUT ${header}
        COMMAND ${WAYLAND_SCANNER} server-header ${xml} ${header}
        DEPENDS ${xml} VERBATIM)
    list(APPEND TEST_PROTOCOL_SOURCES ${code} ${header})
endforeach()

add_executable(lingmo-sdk-wayland-tests
    wayland/connection_test.cpp
    wayland/data_device_test.cpp
    wayland/output_test.cpp
    wayland/seat_test.cpp
    wayland/selection_test.cpp
    wayland/status_item_test.cpp
    wayland/window_hints_test.cpp
    ${TEST_PROTOCOL_SOURCES}
)
target_include_directories(lingmo-sdk-wayland-tests PRIVATE ${TEST_PROTOCOL_DIR})
target_link_libraries(lingmo-sdk-wayland-tests PRIVATE lingmo-sdk GTest::gtest_main
    PkgConfig::WaylandServer PkgConfig::WaylandClient PkgConfig::Xkbcommon Threads::Threads)

gtest_discover_tests(lingmo-sdk-wayland-tests)
