ecm_setup_version(
   PROJECT
   VARIABLE_PREFIX XCB_IMDKIT
   VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/xcb-imdkit_version.h"
   PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/XCBImdkitConfigVersion.cmake"
   SOVERSION 1
   )

configure_package_config_file(
   "${CMAKE_CURRENT_SOURCE_DIR}/XCBImdkitConfig.cmake.in"
   "${CMAKE_CURRENT_BINARY_DIR}/XCBImdkitConfig.cmake"
   INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
   )

set(XCB_IMDKIT_HEADERS
    imdkit.h ximcommon.h ximproto.h ximproto-gen.h imclient.h encoding.h "${CMAKE_CURRENT_BINARY_DIR}/xcbimdkit_export.h"
)

add_library(xcb-imdkit SHARED
parser.c ximproto.c imdkit.c protocolhandler.c message.c common.c imclient.c clientprotocolhandler.c
encoding.c
xlibi18n/lcCT.c
xlibi18n/lcUTF8.c
xlibi18n/lcCharSet.c
)

generate_export_header(xcb-imdkit BASE_NAME XCBImdkit)

add_library(XCBImdkit::XCBImdkit ALIAS xcb-imdkit)

target_include_directories(xcb-imdkit
                           PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                                  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
                           INTERFACE $<INSTALL_INTERFACE:${CMAKE_INSTALL_FULL_INCLUDEDIR}>)

set_target_properties(xcb-imdkit PROPERTIES
                      VERSION ${XCB_IMDKIT_VERSION}
                      SOVERSION ${XCB_IMDKIT_SOVERSION}
                      C_VISIBILITY_PRESET hidden
                      LINK_FLAGS "-Wl,--no-undefined"
                      EXPORT_NAME XCBImdkit)

target_link_libraries(xcb-imdkit
                      PUBLIC XCB::XCB
                      PRIVATE XCB::UTIL UTHash::UTHash)

configure_file(xcb-imdkit.pc.in ${CMAKE_CURRENT_BINARY_DIR}/xcb-imdkit.pc @ONLY)

install(TARGETS xcb-imdkit EXPORT XCBImdkitLibraryTargets LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/xcb-imdkit.pc
  DESTINATION "${CMAKE_INSTALL_LIBDATADIR}/pkgconfig")
install(FILES ${XCB_IMDKIT_HEADERS} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/xcb-imdkit")


install (
   EXPORT XCBImdkitLibraryTargets
   DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
   FILE XCBImdkitLibraryTargets.cmake
   NAMESPACE XCBImdkit::
   )

install (
   FILES "${CMAKE_CURRENT_BINARY_DIR}/XCBImdkitConfig.cmake"
         "${CMAKE_CURRENT_BINARY_DIR}/XCBImdkitConfigVersion.cmake"
   DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
   COMPONENT Devel
   )
