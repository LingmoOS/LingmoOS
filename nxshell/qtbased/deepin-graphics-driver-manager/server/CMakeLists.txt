
set(BIN_NAME deepin-graphics-driver-manager-service)


# Defines path to resource directories
if (CMAKE_BUILD_TYPE MATCHES Debug)
  add_definitions("-DSCRIPTS_DIR=\"${CMAKE_SOURCE_DIR}/server/scripts/\"")
  add_definitions("-DRESOLUTIONS_DIR=\"${CMAKE_SOURCE_DIR}/server/resources/config/resolutions/\"")
else()
  add_definitions("-DSCRIPTS_DIR=\"${CMAKE_INSTALL_PREFIX}/lib/deepin-graphics-driver-manager/\"")
  add_definitions("-DRESOLUTIONS_DIR=\"${CMAKE_INSTALL_PREFIX}/lib/deepin-graphics-driver-manager/\"")
  add_definitions("-DRESOLUTIONS_DIR=\"${CMAKE_INSTALL_PREFIX}/lib/deepin-graphics-driver-manager/\"")
endif()


find_package(PkgConfig REQUIRED)
find_package(Qt5DBus REQUIRED)

pkg_check_modules(PCI REQUIRED libpci)
pkg_check_modules(DtkWidget REQUIRED dtkwidget)

file(GLOB SRCS "*.h" "*.cpp")
add_executable(${BIN_NAME} ${SRCS})

target_include_directories(${BIN_NAME} PUBLIC
    ${DtkWidget_INCLUDE_DIRS}
    ${PCI_INCLUDE_DIRS}
    ${Qt5DBus_INCLUDE_DIRS}
)

target_link_libraries(${BIN_NAME}
    ${PCI_LIBRARIES}
    ${DtkWidget_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
)

# bin
install(TARGETS ${BIN_NAME} DESTINATION lib/deepin-daemon)

# dbus conf
install(FILES data/com.deepin.graphicmanger.conf
        DESTINATION share/dbus-1/system.d/)

#dbus service
install(FILES data/com.deepin.graphicmanger.service
        DESTINATION share/dbus-1/system-services/)

# initramfs-tools
install(PROGRAMS data/deepin-overlay 
        DESTINATION share/initramfs-tools/scripts/init-bottom/)
install(PROGRAMS data/zzz-overlay
        DESTINATION share/initramfs-tools/scripts/init-bottom/)


# resolutions.json
install(FILES resources/config/resolutions/resolutions.json
        DESTINATION deepin-graphics-driver-manager)

# systemd servies
install(FILES resources/service/dgradvrmgr-gltest.service
        DESTINATION /lib/systemd/system)

# scripts
install(DIRECTORY scripts/
        DESTINATION lib/deepin-graphics-driver-manager
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

# resolutions.json
install(FILES resources/config/resolutions/resolutions.json
        DESTINATION lib/deepin-graphics-driver-manager)
