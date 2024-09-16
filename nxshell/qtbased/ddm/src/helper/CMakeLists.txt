include(CheckLibraryExists)

include_directories(
    ${LIBXAU_INCLUDE_DIRS}
)

set(HELPER_SOURCES
    Backend.cpp
    HelperApp.cpp
    UserSession.cpp
    backend/PamHandle.cpp
    backend/PamBackend.cpp
)

add_executable(ddm-helper ${HELPER_SOURCES})
target_link_libraries(ddm-helper
    PRIVATE
        auth
        ${PAM_LIBRARIES}
    PUBLIC
        Qt${QT_MAJOR_VERSION}::Network
        Qt${QT_MAJOR_VERSION}::DBus
        Qt${QT_MAJOR_VERSION}::Qml
)
if("${CMAKE_SYSTEM_NAME}" STREQUAL "FreeBSD")
    # On FreeBSD (possibly other BSDs as well), we want to use
    # setusercontext() to set up the login configuration from login.conf
    find_library(_have_libutil util)
    if(_have_libutil)
        check_library_exists(${_have_libutil} setusercontext "" _have_setusercontext)
    endif()
    if(_have_libutil AND _have_setusercontext)
        target_link_libraries(ddm-helper PUBLIC ${_have_libutil})
    endif()
endif()

install(TARGETS ddm-helper RUNTIME DESTINATION "${CMAKE_INSTALL_LIBEXECDIR}")

add_executable(ddm-helper-start-wayland HelperStartWayland.cpp waylandsocketwatcher.cpp waylandhelper.cpp)
target_link_libraries(ddm-helper-start-wayland
    PRIVATE
        auth
    PUBLIC
        Qt${QT_MAJOR_VERSION}::Core
)
install(TARGETS ddm-helper-start-wayland RUNTIME DESTINATION "${CMAKE_INSTALL_LIBEXECDIR}")

add_executable(ddm-helper-start-single-wayland HelperStartSingleMode.cpp singlewaylandhelper.cpp)
target_link_libraries(ddm-helper-start-single-wayland
    PRIVATE
        auth
    PUBLIC
        Qt${QT_MAJOR_VERSION}::Core
)
install(TARGETS ddm-helper-start-single-wayland RUNTIME DESTINATION "${CMAKE_INSTALL_LIBEXECDIR}")

add_executable(ddm-helper-start-x11user HelperStartX11User.cpp xorguserhelper.cpp)
target_link_libraries(ddm-helper-start-x11user
    PRIVATE
        auth
    PUBLIC
        Qt${QT_MAJOR_VERSION}::Core
)
install(TARGETS ddm-helper-start-x11user RUNTIME DESTINATION "${CMAKE_INSTALL_LIBEXECDIR}")

if(JOURNALD_FOUND)
    target_link_libraries(ddm-helper
        PUBLIC
            ${JOURNALD_LIBRARIES}
    )
    target_link_libraries(ddm-helper-start-x11user
        PUBLIC
            ${JOURNALD_LIBRARIES}
    )
    target_link_libraries(ddm-helper-start-wayland
        PUBLIC
            ${JOURNALD_LIBRARIES}
    )
    target_link_libraries(ddm-helper-start-single-wayland
        PUBLIC
            ${JOURNALD_LIBRARIES}
    )
endif()
