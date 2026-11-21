# SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
# SPDX-FileCopyrightText: 2026 LingmoOS Contributors
# SPDX-License-Identifier: GPL-3.0-or-later

find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(video-player-backend-tests
    main.cpp
    backend_test.cpp
    mpris_test.cpp
)
target_link_libraries(video-player-backend-tests PRIVATE player-backend GTest::gtest)
gtest_discover_tests(video-player-backend-tests)

# Real playback through libmpv on clips ffmpeg makes for the test.
add_executable(video-player-media-tests
    main.cpp
    media_test.cpp
)
target_link_libraries(video-player-media-tests PRIVATE player-backend GTest::gtest)
gtest_discover_tests(video-player-media-tests DISCOVERY_TIMEOUT 30 PROPERTIES TIMEOUT 120)

add_executable(video-player-capture capture.cpp)
target_link_libraries(video-player-capture PRIVATE player-ui)

add_executable(video-player-ui-tests ui_test.cpp)
target_link_libraries(video-player-ui-tests PRIVATE player-ui GTest::gtest)
gtest_discover_tests(video-player-ui-tests DISCOVERY_TIMEOUT 60 PROPERTIES TIMEOUT 180)
