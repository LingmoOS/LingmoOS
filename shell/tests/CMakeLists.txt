add_compile_options(-include${CMAKE_CURRENT_SOURCE_DIR}/namespace_compat.h)

find_package(GTest)
if(NOT GTest_FOUND)
    message(FATAL_ERROR "BUILD_TESTING=ON needs GoogleTest (Debian: libgtest-dev)")
endif()
pkg_check_modules(WaylandClient REQUIRED IMPORTED_TARGET wayland-client)
find_program(DBUS_RUN_SESSION dbus-run-session REQUIRED)

# loop_test_util.h: waiting on the main loop in every test.
add_library(lingmo-test-loop STATIC loop_test_util.cpp)
target_link_libraries(lingmo-test-loop PUBLIC lingmo-core)

# lingmo_add_test(name [DBUS] libraries...)
# A GoogleTest binary on gtest_toolkit_main.cpp, whose main loop runs the
# timers, fd watches, posted tasks and sd-bus connections of the code under
# test while it waits (loop_test_util.h). DBUS runs it on a private session
# bus.
function(lingmo_add_test name)
    cmake_parse_arguments(PARSE_ARGV 1 arg "DBUS" "" "")
    add_executable(${name} ${name}.cpp gtest_toolkit_main.cpp)
    target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(${name} PRIVATE GTest::gtest lingmo-core lingmo-test-loop
        ${arg_UNPARSED_ARGUMENTS})
    if(arg_DBUS)
        add_test(NAME ${name} COMMAND ${DBUS_RUN_SESSION} -- $<TARGET_FILE:${name}>)
    else()
        add_test(NAME ${name} COMMAND ${name})
    endif()
endfunction()

lingmo_add_test(main_loop_test lingmo-sdk-host)
lingmo_add_test(four_finger_pinch_test lingmo-wm)
lingmo_add_test(core_test)
target_sources(core_test PRIVATE ${PROJECT_SOURCE_DIR}/src/core/compositor_options.cpp)
target_compile_definitions(core_test PRIVATE
    LINGMO_PORTAL_BACKEND="${CMAKE_INSTALL_FULL_LIBEXECDIR}/xdg-desktop-portal-lingmo"
    LINGMO_SCREEN_RECORDER="${CMAKE_INSTALL_FULL_LIBEXECDIR}/lingmo-screenrecorder"
    LINGMO_BINDIR="${CMAKE_INSTALL_FULL_BINDIR}")

lingmo_add_test(image_capture_test lingmo-aurora-core lingmo-session PkgConfig::WaylandClient)
lingmo_add_test(serial_tracker_test lingmo-wayland)
lingmo_add_test(client_tracker_test lingmo-wayland)
lingmo_add_test(wayland_server_test lingmo-wayland PkgConfig::WaylandClient)
lingmo_add_test(scene_test lingmo-scene)
lingmo_add_test(shm_pool_test lingmo-aurora-core)
lingmo_add_test(buffer_test lingmo-aurora-core)
lingmo_add_test(surface_test lingmo-aurora-core)
lingmo_add_test(wayland_protocol_test lingmo-aurora-core lingmo-wayland-dmabuf
    PkgConfig::WaylandClient)
lingmo_add_test(xkb_tracker_test lingmo-input)
lingmo_add_test(key_translator_test lingmo-input)
lingmo_add_test(input_test lingmo-input)
lingmo_add_test(computer_control_test lingmo-computer-control)
lingmo_add_test(remembered_apps_test lingmo-computer-control)
lingmo_add_test(touch_test lingmo-input)
foreach(test pointer_acceleration_test touchpad_click_test)
    add_executable(${test} ${test}.cpp)
    target_link_libraries(${test} PRIVATE lingmo-mouse-settings GTest::gtest_main)
    add_test(NAME ${test} COMMAND ${test})
endforeach()
add_executable(mouse_settings_test mouse_settings_test.cpp)
target_include_directories(mouse_settings_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mouse_settings_test PRIVATE lingmo-mouse-settings GTest::gtest_main)
add_test(NAME mouse_settings_test COMMAND ${DBUS_RUN_SESSION} -- $<TARGET_FILE:mouse_settings_test>)
add_executable(touchpad_settings_test touchpad_settings_test.cpp)
target_include_directories(touchpad_settings_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(touchpad_settings_test PRIVATE lingmo-mouse-settings GTest::gtest_main)
add_test(NAME touchpad_settings_test COMMAND ${DBUS_RUN_SESSION} -- $<TARGET_FILE:touchpad_settings_test>)
add_executable(touchscreen_settings_test touchscreen_settings_test.cpp)
target_include_directories(touchscreen_settings_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(touchscreen_settings_test PRIVATE lingmo-mouse-settings GTest::gtest_main)
add_test(NAME touchscreen_settings_test COMMAND ${DBUS_RUN_SESSION} -- $<TARGET_FILE:touchscreen_settings_test>)
add_executable(keyboard_settings_test keyboard_settings_test.cpp)
target_include_directories(keyboard_settings_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(keyboard_settings_test PRIVATE lingmo-keyboard-settings GTest::gtest_main)
add_test(NAME keyboard_settings_test COMMAND ${DBUS_RUN_SESSION} -- $<TARGET_FILE:keyboard_settings_test>)
lingmo_add_test(drm_selection_test lingmo-backend)
lingmo_add_test(drm_properties_test lingmo-backend)
lingmo_add_test(drm_configuration_planner_test lingmo-fake-drm)
lingmo_add_test(drm_multi_device_planner_test lingmo-backend)
lingmo_add_test(drm_hotplug_monitor_test lingmo-backend)
lingmo_add_test(libseat_session_test lingmo-backend)

# Manual wl_shm client. Not registered with ctest: it needs a running
# compositor and a screen to look at.
add_executable(lingmo-shm-demo manual/shm_client.cpp)
target_link_libraries(lingmo-shm-demo PRIVATE PkgConfig::WaylandClient)

lingmo_add_test(sub_surface_test lingmo-aurora-core)
lingmo_add_test(wm_test lingmo-wm)
lingmo_add_test(subsurface_output_protocol_test lingmo-aurora-core PkgConfig::WaylandClient)
lingmo_add_test(seat_protocol_test lingmo-wayland-input lingmo-wm
    PkgConfig::WaylandClient PkgConfig::Xkbcommon)
lingmo_add_test(xdg_positioner_test lingmo-wayland-shell)
lingmo_add_test(shell_surface_test lingmo-decoration)
lingmo_add_test(decoration_test lingmo-decoration)
lingmo_add_test(text_test lingmo-decoration PkgConfig::TitleText)
lingmo_add_test(xdg_shell_protocol_test lingmo-wayland-shell lingmo-wayland-input
    lingmo-wayland-private-shell PkgConfig::WaylandClient)
lingmo_add_test(status_item_protocol_test lingmo-wayland-shell lingmo-wayland-input
    PkgConfig::WaylandClient)
lingmo_add_test(protocol_binding_test lingmo-wayland-globals PkgConfig::WaylandClient)
set_tests_properties(protocol_binding_test PROPERTIES TIMEOUT 60)
# Seeded random request sequences against every global; with clang,
# -DLINGMO_LIBFUZZER=ON builds it for libFuzzer and AddressSanitizer instead.
option(LINGMO_LIBFUZZER "Build protocol_fuzzer for libFuzzer" OFF)
add_executable(protocol_fuzzer protocol_fuzzer.cpp)
target_include_directories(protocol_fuzzer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(protocol_fuzzer PRIVATE lingmo-wayland-globals PkgConfig::WaylandClient)
if(LINGMO_LIBFUZZER)
    target_compile_definitions(protocol_fuzzer PRIVATE LINGMO_LIBFUZZER)
    target_compile_options(protocol_fuzzer PRIVATE -fsanitize=fuzzer,address)
    target_link_options(protocol_fuzzer PRIVATE -fsanitize=fuzzer,address)
else()
    add_test(NAME protocol_fuzzer COMMAND protocol_fuzzer)
    set_tests_properties(protocol_fuzzer PROPERTIES TIMEOUT 300)
endif()
lingmo_add_test(xdg_foreign_protocol_test lingmo-wayland-shell lingmo-wayland-input
    PkgConfig::WaylandClient)
lingmo_add_test(xdg_toplevel_drag_protocol_test lingmo-wayland-shell lingmo-wayland-input
    PkgConfig::WaylandClient)
lingmo_add_test(foreign_toplevel_list_test lingmo-wayland-private-shell lingmo-input
    lingmo-aurora-core PkgConfig::WaylandClient)
lingmo_add_test(private_shell_protocol_test lingmo-wayland-private-shell
    lingmo-wayland-shell lingmo-wayland-input PkgConfig::WaylandClient)
lingmo_add_test(work_area_test lingmo-wm)
lingmo_add_test(dock_layout_test lingmo-wm)
lingmo_add_test(session_lock_test lingmo-session lingmo-wm lingmo-scene lingmo-decoration)
lingmo_add_test(accelerator_test DBUS lingmo-accelerators lingmo-session lingmo-wm
    lingmo-scene)
lingmo_add_test(xdg_decoration_protocol_test lingmo-wayland-shell lingmo-wayland-input
    lingmo-decoration PkgConfig::WaylandClient)
add_executable(lingmo-subsurface-demo manual/subsurface_client.cpp)
target_link_libraries(lingmo-subsurface-demo PRIVATE PkgConfig::WaylandClient)

# Manual xdg-shell demo. --script runs the whole toplevel state machine
# and exits, which is what the VT harness drives.
add_executable(lingmo-xdg-demo manual/xdg_client.cpp)
target_link_libraries(lingmo-xdg-demo PRIVATE PkgConfig::WaylandClient lingmo-protocols)

# Manual xdg-decoration demo. Draws only its client area, so everything
# around it on screen is the compositor's own frame.
add_executable(lingmo-decoration-demo manual/decoration_client.cpp)
target_link_libraries(lingmo-decoration-demo
    PRIVATE PkgConfig::WaylandClient lingmo-protocols)

# Development harness that plays the session's role for the shell: it obtains a
# privileged Wayland connection from a running compositor and execs a program
# with it. Not registered with ctest; it needs a compositor and a shell.
add_executable(lingmo-shell-session-launcher manual/shell_session_launcher.cpp)

# Manual input demo. Needs a running compositor, a screen and a mouse.
add_executable(lingmo-input-demo manual/input_client.cpp)
target_link_libraries(lingmo-input-demo PRIVATE PkgConfig::WaylandClient PkgConfig::Xkbcommon)
target_compile_definitions(subsurface_output_protocol_test PRIVATE
    SUBSURFACE_DEMO_PATH="$<TARGET_FILE:lingmo-subsurface-demo>")
add_dependencies(subsurface_output_protocol_test lingmo-subsurface-demo)

lingmo_add_test(output_scheduler_test lingmo-output-scheduler lingmo-aurora-core lingmo-decoration)

lingmo_add_test(clipboard_protocol_test lingmo-wayland-input PkgConfig::WaylandClient)
set_tests_properties(clipboard_protocol_test PROPERTIES TIMEOUT 30)

lingmo_add_test(selection_test lingmo-input)

lingmo_add_test(dmabuf_test lingmo-wayland-dmabuf lingmo-aurora-core
    PkgConfig::WaylandClient PkgConfig::Libdrm)

lingmo_add_test(shadow_test lingmo-decoration)
lingmo_add_test(shadow_controller_test lingmo-decoration)
lingmo_add_test(shadow_pixel_test lingmo-decoration lingmo-render)
set_tests_properties(shadow_pixel_test PROPERTIES ENVIRONMENT "LIBGL_ALWAYS_SOFTWARE=1;EGL_PLATFORM=surfaceless")

lingmo_add_test(motion_test lingmo-scene)
lingmo_add_test(window_animation_test lingmo-scene)
lingmo_add_test(screen_transition_node_test lingmo-scene)
lingmo_add_test(desk_switch_node_test lingmo-scene)
lingmo_add_test(desks_test lingmo-wm)
lingmo_add_test(overview_layout_test lingmo-wm)
lingmo_add_test(overview_node_test lingmo-scene)
lingmo_add_test(overview_test lingmo-wm)
lingmo_add_test(show_desktop_test lingmo-wm)
lingmo_add_test(hot_corner_test lingmo-accelerators)

lingmo_add_test(cursor_test lingmo-cursor)
lingmo_add_test(output_cursor_router_test lingmo-cursor)

# DMA-BUF scanout: eligibility, plane assignment, the atomic commit
# path, explicit synchronization and the hardware cursor.
lingmo_add_test(overlay_candidate_test lingmo-scene)
lingmo_add_test(plane_assignment_test lingmo-backend PkgConfig::Libdrm)
lingmo_add_test(sync_fence_test lingmo-sync lingmo-aurora-core)
lingmo_add_test(commit_readiness_test lingmo-sync lingmo-aurora-core)

add_library(lingmo-fake-drm STATIC fake_drm_device.cpp fake_drm_device.h)
target_include_directories(lingmo-fake-drm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(lingmo-fake-drm PUBLIC lingmo-backend PkgConfig::Libdrm)

lingmo_add_test(scanout_test lingmo-fake-drm lingmo-aurora-core)
lingmo_add_test(hardware_cursor_test lingmo-fake-drm lingmo-cursor lingmo-session)

lingmo_add_test(window_hints_protocol_test lingmo-wayland-shell PkgConfig::WaylandClient)
lingmo_add_test(surface_blur_protocol_test lingmo-wayland-shell PkgConfig::WaylandClient)

lingmo_add_test(dnd_protocol_test lingmo-cursor lingmo-wayland-input
    lingmo-wayland-private-shell PkgConfig::WaylandClient)

lingmo_add_test(data_control_protocol_test lingmo-wayland-input lingmo-wayland-private-shell
    lingmo-wayland-shell PkgConfig::WaylandClient)

lingmo_add_test(dnd_toolkit_test lingmo-wayland-input lingmo-wayland-shell PkgConfig::WaylandClient)
target_compile_definitions(dnd_toolkit_test PRIVATE GTK_DND_CLIENT="${CMAKE_CURRENT_SOURCE_DIR}/dnd/gtk_client.py")
set_tests_properties(dnd_toolkit_test PROPERTIES TIMEOUT 70)

set_tests_properties(private_shell_protocol_test PROPERTIES TIMEOUT 60)

lingmo_add_test(viewporter_test lingmo-aurora-core)
lingmo_add_test(viewporter_protocol_test lingmo-aurora-core PkgConfig::WaylandClient)
set_tests_properties(viewporter_protocol_test PROPERTIES TIMEOUT 30)
lingmo_add_test(viewporter_pixel_test lingmo-aurora-core lingmo-input lingmo-render
    PkgConfig::Gbm PkgConfig::Libdrm)
set_tests_properties(viewporter_pixel_test PROPERTIES ENVIRONMENT "LIBGL_ALWAYS_SOFTWARE=1;EGL_PLATFORM=surfaceless")

lingmo_add_test(fractional_scale_protocol_test lingmo-aurora-core PkgConfig::WaylandClient)
set_tests_properties(fractional_scale_protocol_test PROPERTIES TIMEOUT 30)

lingmo_add_test(fractional_scale_pixel_test lingmo-aurora-core lingmo-render)
set_tests_properties(fractional_scale_pixel_test PROPERTIES ENVIRONMENT "LIBGL_ALWAYS_SOFTWARE=1;EGL_PLATFORM=surfaceless")

lingmo_add_test(output_scale_test lingmo-input)
lingmo_add_test(display_types_test lingmo-display)
lingmo_add_test(edid_test lingmo-display)
lingmo_add_test(display_preferences_test lingmo-display)
lingmo_add_test(recommended_scale_test lingmo-display)
lingmo_add_test(display_transform_test lingmo-display)
lingmo_add_test(mirror_transform_test lingmo-display)
lingmo_add_test(display_layout_test lingmo-display)
lingmo_add_test(chromeos_display_layout_test lingmo-display)
lingmo_add_test(display_configuration_validator_test lingmo-display)
lingmo_add_test(display_manager_test lingmo-display)
lingmo_add_test(native_display_delegate_test lingmo-display)
target_sources(native_display_delegate_test PRIVATE
    fake_native_display_delegate.cpp fake_native_display_delegate.h)
lingmo_add_test(display_configuration_controller_test lingmo-display)
target_sources(display_configuration_controller_test PRIVATE
    fake_native_display_delegate.cpp fake_native_display_delegate.h)
lingmo_add_test(night_light_test lingmo-display)
lingmo_add_test(display_config_service_test DBUS lingmo-display-ipc)
target_sources(display_config_service_test PRIVATE
    fake_native_display_delegate.cpp fake_native_display_delegate.h)

lingmo_add_test(presentation_test lingmo-aurora-core)
lingmo_add_test(presentation_protocol_test lingmo-aurora-core PkgConfig::WaylandClient)
add_executable(lingmo-presentation-client manual/presentation_client.cpp)
target_link_libraries(lingmo-presentation-client PRIVATE lingmo-protocols PkgConfig::WaylandClient)

lingmo_add_test(drm_output_test lingmo-backend lingmo-aurora-core PkgConfig::WaylandClient)
target_sources(drm_output_test PRIVATE fake_drm_device.cpp)
set_tests_properties(drm_output_test PROPERTIES TIMEOUT 60 ENVIRONMENT "LIBGL_ALWAYS_SOFTWARE=1;EGL_PLATFORM=surfaceless")

lingmo_add_test(window_visual_test lingmo-decoration)
lingmo_add_test(window_visual_pixel_test lingmo-decoration lingmo-render)
set_tests_properties(window_visual_pixel_test PROPERTIES ENVIRONMENT "LIBGL_ALWAYS_SOFTWARE=1;EGL_PLATFORM=surfaceless")
add_executable(toolkit-resize-client visuals/toolkit_client.cpp)
target_link_libraries(toolkit-resize-client PRIVATE Lingmo::SDK)
# DBUS: the GTK clients activate portal and gvfs services on the session bus.
lingmo_add_test(window_visual_toolkit_test DBUS lingmo-wayland-input lingmo-wayland-shell
    lingmo-decoration lingmo-render PkgConfig::WaylandClient)
target_compile_definitions(window_visual_toolkit_test PRIVATE
    TOOLKIT_RESIZE_CLIENT="$<TARGET_FILE:toolkit-resize-client>"
    ELECTRON_VISUAL_CLIENT="${CMAKE_CURRENT_SOURCE_DIR}/visuals/electron_client.js")
add_dependencies(window_visual_toolkit_test toolkit-resize-client)
set_tests_properties(window_visual_toolkit_test PROPERTIES TIMEOUT 70
    ENVIRONMENT "LIBGL_ALWAYS_SOFTWARE=1;EGL_PLATFORM=surfaceless")

if(LINGMO_ENABLE_XWAYLAND)
    lingmo_add_test(xwayland_test lingmo-xwayland lingmo-wayland-shell lingmo-wayland-input lingmo-render lingmo-decoration PkgConfig::WaylandClient)
    add_executable(xwayland-startup-fault manual/xwayland_startup_fault.cpp)
    target_compile_definitions(xwayland_test PRIVATE XWAYLAND_FAULT_PATH="$<TARGET_FILE:xwayland-startup-fault>")
    add_executable(x11-test-client xwayland/x11_client.cpp)
    target_link_libraries(x11-test-client PRIVATE PkgConfig::Xcb)
    target_compile_definitions(xwayland_test PRIVATE X11_TEST_CLIENT="$<TARGET_FILE:x11-test-client>"
        GTK_DND_CLIENT="${CMAKE_CURRENT_SOURCE_DIR}/dnd/gtk_client.py")
    add_dependencies(xwayland_test xwayland-startup-fault x11-test-client)
    set_tests_properties(xwayland_test PROPERTIES TIMEOUT 90 ENVIRONMENT "LIBGL_ALWAYS_SOFTWARE=1;EGL_PLATFORM=surfaceless")
endif()

lingmo_add_test(display_runtime_test lingmo-display-runtime lingmo-session)

lingmo_add_test(capture_pixel_test lingmo-render)
lingmo_add_test(render_internals_test lingmo-render PkgConfig::Gbm PkgConfig::Libdrm)
set_tests_properties(render_internals_test PROPERTIES ENVIRONMENT "LIBGL_ALWAYS_SOFTWARE=1;EGL_PLATFORM=surfaceless")
set_tests_properties(capture_pixel_test PROPERTIES ENVIRONMENT "LIBGL_ALWAYS_SOFTWARE=1;EGL_PLATFORM=surfaceless")
lingmo_add_test(desk_switch_pixel_test lingmo-scene lingmo-render)
set_tests_properties(desk_switch_pixel_test PROPERTIES ENVIRONMENT "LIBGL_ALWAYS_SOFTWARE=1;EGL_PLATFORM=surfaceless")
lingmo_add_test(overview_pixel_test lingmo-scene lingmo-render)
set_tests_properties(overview_pixel_test PROPERTIES ENVIRONMENT "LIBGL_ALWAYS_SOFTWARE=1;EGL_PLATFORM=surfaceless")
lingmo_add_test(wallpaper_pixel_test lingmo-scene lingmo-render)
set_tests_properties(wallpaper_pixel_test PROPERTIES ENVIRONMENT "LIBGL_ALWAYS_SOFTWARE=1;EGL_PLATFORM=surfaceless")
set_tests_properties(image_capture_test PROPERTIES TIMEOUT 15)


lingmo_add_test(damage_test lingmo-aurora-core)
lingmo_add_test(damage_pixel_test lingmo-scene lingmo-render)
set_tests_properties(damage_pixel_test PROPERTIES ENVIRONMENT "LIBGL_ALWAYS_SOFTWARE=1;EGL_PLATFORM=surfaceless")
lingmo_add_test(damage_gbm_test lingmo-scene lingmo-render PkgConfig::Gbm)
lingmo_add_test(dmabuf_format_gbm_test lingmo-render PkgConfig::Gbm)
lingmo_add_test(capture_dmabuf_gbm_test lingmo-render PkgConfig::Gbm PkgConfig::Libdrm)

lingmo_add_test(idle_notify_protocol_test lingmo-wayland-input PkgConfig::WaylandClient)

lingmo_add_test(auth_controller_test lingmo-auth)
lingmo_add_test(switcher_view_test lingmo-switcher)
lingmo_add_test(shell_test lingmo-shell lingmo-computer-control lingmo-session
    lingmo-wayland-input)
target_sources(shell_test PRIVATE ${PROJECT_SOURCE_DIR}/src/core/computer_control_environment.cpp)
lingmo_add_test(shell_gpu_pixel_test lingmo-shell lingmo-render)
set_tests_properties(shell_gpu_pixel_test PROPERTIES ENVIRONMENT "LIBGL_ALWAYS_SOFTWARE=1;EGL_PLATFORM=surfaceless")
# The shell components' memory, step by step; by hand.
add_executable(lingmo-shell-memory-bench manual/shell_memory_bench.cpp)
target_include_directories(lingmo-shell-memory-bench PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(lingmo-shell-memory-bench PRIVATE lingmo-shell-osd lingmo-shell-dock-view
    lingmo-decoration lingmo-render lingmo-test-loop)
lingmo_add_test(panel_gpu_pixel_test lingmo-auth lingmo-render)
target_include_directories(panel_gpu_pixel_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
set_tests_properties(panel_gpu_pixel_test PROPERTIES ENVIRONMENT "LIBGL_ALWAYS_SOFTWARE=1;EGL_PLATFORM=surfaceless")
lingmo_add_test(switcher_gpu_pixel_test lingmo-switcher lingmo-render)
set_tests_properties(switcher_gpu_pixel_test PROPERTIES ENVIRONMENT "LIBGL_ALWAYS_SOFTWARE=1;EGL_PLATFORM=surfaceless")
lingmo_add_test(auth_ui_test DBUS lingmo-auth)
lingmo_add_test(greetd_backend_test lingmo-auth)
lingmo_add_test(screen_saver_service_test DBUS lingmo-session)
lingmo_add_test(applications_service_test DBUS lingmo-session)

lingmo_add_test(window_debug_service_test DBUS lingmo-session)

lingmo_add_test(sleep_lock_test DBUS lingmo-session)

lingmo_add_test(power_button_test DBUS lingmo-session)

lingmo_add_test(input_method_protocol_test lingmo-wayland-input PkgConfig::WaylandClient)
set_tests_properties(input_method_protocol_test PROPERTIES TIMEOUT 45)

add_executable(input_method_settings_test input_method_settings_test.cpp)
target_include_directories(input_method_settings_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(input_method_settings_test PRIVATE lingmo-input-method-settings
    GTest::gtest_main)
add_test(NAME input_method_settings_test
    COMMAND ${DBUS_RUN_SESSION} -- $<TARGET_FILE:input_method_settings_test>)

add_executable(input_method_launched_client input_method/launched_client.cpp)
target_link_libraries(input_method_launched_client PRIVATE PkgConfig::WaylandClient)
lingmo_add_test(input_method_launcher_test DBUS lingmo-wayland-input
    lingmo-wayland-private-shell PkgConfig::WaylandClient)
target_compile_definitions(input_method_launcher_test PRIVATE
    LINGMO_LAUNCHED_CLIENT="$<TARGET_FILE:input_method_launched_client>")
add_dependencies(input_method_launcher_test input_method_launched_client)
set_tests_properties(input_method_launcher_test PROPERTIES TIMEOUT 60)

# Sanitizer builds run several times slower than the per-test timeouts above
# allow for; configure with -DLINGMO_TEST_TIMEOUT_SCALE=5 for those.
set(LINGMO_TEST_TIMEOUT_SCALE 1 CACHE STRING "Multiplier for every test's TIMEOUT")
if(NOT LINGMO_TEST_TIMEOUT_SCALE EQUAL 1)
    get_directory_property(lingmo_all_tests TESTS)
    foreach(test ${lingmo_all_tests})
        get_test_property(${test} TIMEOUT timeout)
        if(timeout AND NOT timeout STREQUAL "NOTFOUND")
            math(EXPR scaled "${timeout} * ${LINGMO_TEST_TIMEOUT_SCALE}")
            set_tests_properties(${test} PROPERTIES TIMEOUT ${scaled})
        endif()
    endforeach()
endif()

add_subdirectory(shell)

# Also buildable on its own against the SDK, without a DRM/GPU backend.
add_subdirectory(dock-magnification)

add_subdirectory(cursor-shake)
