project(dde-keyring-checker LANGUAGES C)

set(BIN_NAME dde-keyring-checker)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SECRET1 REQUIRED IMPORTED_TARGET libsecret-1)

message(${PROJECT_SOURCE_DIR})

add_executable(${BIN_NAME}
    main.c
    )

target_link_libraries(${BIN_NAME}
    PkgConfig::SECRET1
    )

install(TARGETS ${BIN_NAME} DESTINATION bin)

set(SYSTEMD_FILES
    dde-keyring-checker.service
)
install(FILES ${SYSTEMD_FILES} DESTINATION lib/systemd/user/)
install(DIRECTORY DESTINATION lib/systemd/user/dde-session-initialized.target.wants/)
install_symlink(dde-keyring-checker.service dde-session-initialized.target.wants)
