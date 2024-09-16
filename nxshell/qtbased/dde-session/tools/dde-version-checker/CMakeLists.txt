set(BIN_NAME dde-version-checker)

#find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS Core REQUIRED)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

message(${PROJECT_SOURCE_DIR})

add_executable(${BIN_NAME}
    main.cpp
    )

target_link_libraries(${BIN_NAME}
    Qt${QT_VERSION_MAJOR}::Core
    )

install(TARGETS ${BIN_NAME} DESTINATION bin)

set(SYSTEMD_FILES
    dde-version-checker.service
)
install(FILES ${SYSTEMD_FILES} DESTINATION lib/systemd/user/)
install(DIRECTORY DESTINATION lib/systemd/user/dde-session-initialized.target.wants/)
install_symlink(dde-version-checker.service dde-session-initialized.target.wants)
