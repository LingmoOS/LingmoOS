include_directories( ${UNIXAUTH_INCLUDE_DIRS} )
check_include_files(paths.h HAVE_PATHS_H)
configure_file (config-ccheckpass.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ccheckpass.h )

set(ccheckpass_SRCS
	checkpass.h
	checkpass.c
	checkpass_pam.c
	checkpass_shadow.c
)

# find_package(ECM REQUIRED NO_MODULE)
# set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} )
# include(ECMMarkNonGuiExecutable)

add_executable(ccheckpass ${ccheckpass_SRCS})
# ecm_mark_nongui_executable(ccheckpass)

set_property(TARGET ccheckpass APPEND_STRING PROPERTY COMPILE_FLAGS " -U_REENTRANT")
target_link_libraries(ccheckpass ${UNIXAUTH_LIBRARIES} ${SOCKET_LIBRARIES})

if (PAM_FOUND)
    set(checkpass_suid "")
else()
    set(checkpass_suid "SETUID")
    message(STATUS "PAM not found, will install a SUID-kcheckpass")
endif()
install(TARGETS ccheckpass
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE ${checkpass_suid}
    )

#EXTRA_DIST = README
