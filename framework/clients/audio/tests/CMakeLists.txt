find_package(GTest REQUIRED)
find_program(PIPEWIRE_EXECUTABLE pipewire REQUIRED)
find_program(WIREPLUMBER_EXECUTABLE wireplumber REQUIRED)
find_program(PW_CLI_EXECUTABLE pw-cli REQUIRED)

add_executable(audio-client-tests audio_test.cpp)
set_target_properties(audio-client-tests PROPERTIES AUTOMOC OFF AUTORCC OFF)
target_link_libraries(audio-client-tests PRIVATE Lingmo::AudioClient GTest::gtest_main)
target_compile_definitions(audio-client-tests PRIVATE
    AUDIO_TEST_CONFIG="${CMAKE_CURRENT_SOURCE_DIR}/pipewire.conf"
    WIREPLUMBER_TEST_CONFIG="${CMAKE_CURRENT_SOURCE_DIR}/wireplumber-test.conf"
    PIPEWIRE_EXECUTABLE="${PIPEWIRE_EXECUTABLE}"
    WIREPLUMBER_EXECUTABLE="${WIREPLUMBER_EXECUTABLE}"
    PW_CLI_EXECUTABLE="${PW_CLI_EXECUTABLE}"
)
# WirePlumber wants a session bus; the private daemons stay out of the live session's.
add_test(NAME audio-client-tests COMMAND dbus-run-session -- $<TARGET_FILE:audio-client-tests>)
set_tests_properties(audio-client-tests PROPERTIES TIMEOUT 120)
