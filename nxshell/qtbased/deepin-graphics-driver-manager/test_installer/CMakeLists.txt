
set(BIN_NAME test-installer)

if (CMAKE_BUILD_TYPE MATCHES Debug)
  add_definitions("-DTRANSLATIONS_DIR=\"${CMAKE_BINARY_DIR}/translations/\"")
  add_definitions("-DRESOURCES_DIR=\"${CMAKE_SOURCE_DIR}/test-installer/resources\"")
else()
  add_definitions("-DTRANSLATIONS_DIR=\"${CMAKE_INSTALL_PREFIX}/share/deepin-graphics-driver-manager/translations/\"")
  add_definitions("-DRESOURCES_DIR=\"${CMAKE_INSTALL_PREFIX}/share/deepin-graphics-driver-manager/test-installer/\"")
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
    ${Qt5DBus_INCLUDE_DIRS})

target_link_libraries(${BIN_NAME}
    ${Qt5DBus_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    ${DtkWidget_LIBRARIES}
)

install(TARGETS ${BIN_NAME} DESTINATION lib/deepin-graphics-driver-manager)
install(FILES deepin-gradvrmgr-test-installer.desktop DESTINATION lib/deepin-graphics-driver-manager)


