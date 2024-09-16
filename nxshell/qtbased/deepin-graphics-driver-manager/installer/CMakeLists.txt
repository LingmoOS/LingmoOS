add_executable(installer main.cpp ${UTILS_FILES} resources.qrc)

find_package(PkgConfig REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5DBus REQUIRED)

if (CMAKE_BUILD_TYPE MATCHES Debug)
  add_definitions("-DTRANSLATIONS_DIR=\"${CMAKE_BINARY_DIR}/translations/\"")
else()
  add_definitions("-DTRANSLATIONS_DIR=\"${CMAKE_INSTALL_PREFIX}/share/deepin-graphics-driver-manager/translations/\"")
endif()


find_package(DtkCMake REQUIRED)
find_package(DtkWidget REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/utils/)

target_include_directories(installer PUBLIC
    ${DtkWidget_INCLUDE_DIRS}
    ${Qt5DBus_INCLUDE_DIRS}
)

target_link_libraries(installer
    ${Qt5Widgets_LIBRARIES}
    ${DtkWidget_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
)


install(TARGETS installer DESTINATION lib/deepin-graphics-driver-manager)
install(FILES deepin-gradvrmgr-installer.desktop DESTINATION lib/deepin-graphics-driver-manager)
