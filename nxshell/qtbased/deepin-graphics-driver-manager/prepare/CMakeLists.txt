
set(BIN_NAME deepin-graphics-driver-manager)

if (CMAKE_BUILD_TYPE MATCHES Debug)
  add_definitions("-DTRANSLATIONS_DIR=\"${CMAKE_BINARY_DIR}/translations/\"")
  add_definitions("-DRESOURCES_DIR=\"${CMAKE_SOURCE_DIR}/prepare/resources\"")
else()
  add_definitions("-DTRANSLATIONS_DIR=\"${CMAKE_INSTALL_PREFIX}/share/deepin-graphics-driver-manager/translations/\"")
  add_definitions("-DRESOURCES_DIR=\"${CMAKE_INSTALL_PREFIX}/share/deepin-graphics-driver-manager/prepare/\"")
endif()

find_package(PkgConfig REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(DtkCMake REQUIRED)
find_package(DtkWidget REQUIRED)

file(GLOB SRCS "*.h" "*.cpp")

include_directories(${CMAKE_SOURCE_DIR}/utils/)
add_executable(${BIN_NAME} ${SRCS} ${UTILS_FILES} resources.qrc)

target_include_directories(${BIN_NAME} PUBLIC
        ${DtkWidget_INCLUDE_DIRS}
        ${Qt5DBus_INCLUDE_DIRS}
        )

target_link_libraries(${BIN_NAME}
        ${Qt5DBus_LIBRARIES}
        ${Qt5Widgets_LIBRARIES}
        ${DtkWidget_LIBRARIES}
        )
add_dependencies(${BIN_NAME} graphics-qm-update)

# bin
install(TARGETS ${BIN_NAME} DESTINATION bin)

# desktop
install(FILES ../deepin-graphics-driver-manager.desktop DESTINATION share/applications)

# icons
foreach (ICON_SIZE 64)
    install(FILES resources/icons/deepin-graphics-driver-manager-${ICON_SIZE}px.svg
            RENAME deepin-graphics-driver-manager.svg
            DESTINATION share/icons/hicolor/${ICON_SIZE}x${ICON_SIZE}/apps)
endforeach ()
install(FILES resources/icons/deepin-graphics-driver-manager-64px.svg
        DESTINATION share/icons/hicolor/scalable/apps)

#test file
file(GLOB TEST_FILE "resources/test/*")
install(FILES ${TEST_FILE}
        DESTINATION share/deepin-graphics-driver-manager/prepare/test/)

# qm files
file(GLOB QM_FILES "../translations/*.qm")
install(DIRECTORY ${CMAKE_BINARY_DIR}/translations
        DESTINATION /usr/share/deepin-graphics-driver-manager/
        FILES_MATCHING PATTERN "*"
)

