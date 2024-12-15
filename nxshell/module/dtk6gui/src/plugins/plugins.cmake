option(DTK_DISABLE_XCB "Disable XCB Protocols" OFF)

option(DTK_DISABLE_TREELAND "Disable Treeland Protocols" OFF)

file(GLOB PLATFORM_INTERFACE_HEADER
    ${CMAKE_CURRENT_LIST_DIR}/*.h
)
file(GLOB PLATFORM_INTERFACE_SOURCE
    ${CMAKE_CURRENT_LIST_DIR}/*.cpp
)

set(platform_interface_SRC 
    ${PLATFORM_INTERFACE_HEADER}
    ${PLATFORM_INTERFACE_SOURCE}
)
target_sources(${LIB_NAME} PRIVATE
    ${platform_interface_SRC}
)

set(PRIVATE_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/private/dplatforminterface_p.h)
set(PRIVATE_INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_INCLUDEDIR}/dtk${PROJECT_VERSION_MAJOR}/DGui/Private")

# XCB
if(NOT DTK_DISABLE_XCB)
    message("Support XCB!")
    include(plugins/platform/xcb/xcb.cmake)
    target_sources(${LIB_NAME} PRIVATE
        ${xcb_SRC}
    )
else()
    target_compile_definitions(${LIB_NAME} PRIVATE DTK_DISABLE_XCB)
endif()

# Treeland

try_compile(LINGMO_WAYLAND_TEST_COMPILE_RESULT ${CMAKE_CURRENT_BINARY_DIR}/treeland_test
    ${CMAKE_CURRENT_LIST_DIR}/platform/config.tests/treeland_test treeland_test CMAKE_FLAGS -DQT_VERSION_MAJOR=${QT_VERSION_MAJOR})

if(NOT LINGMO_WAYLAND_TEST_COMPILE_RESULT)
    message("wayland_test failed, disable treeland support")
    set(DTK_DISABLE_TREELAND ON)
endif()

if(NOT DTK_DISABLE_TREELAND)
    find_package(TreelandProtocols)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS WaylandClient)
    set(TreelandProtocols_FOUND ${TreelandProtocols_FOUND})
endif()

if(NOT DTK_DISABLE_TREELAND AND TreelandProtocols_FOUND)
    message("Support Treeland!")
    if("${QT_VERSION_MAJOR}" STREQUAL "6")
        qt6_generate_wayland_protocol_client_sources(${LIB_NAME} FILES
        ${TREELAND_PROTOCOLS_DATA_DIR}/treeland-personalization-manager-v1.xml
    )
    else()
        # ECM setup
        include(FeatureSummary)
        find_package(ECM REQUIRED NO_MODULE)
        set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${ECM_MODULE_PATH}")
        find_package(QtWaylandScanner)
        ecm_add_qtwayland_client_protocol(CLIENT_LIB_SRCS
            PROTOCOL ${TREELAND_PROTOCOLS_DATA_DIR}/treeland-personalization-manager-v1.xml
            BASENAME treeland-personalization-manager-v1
        )
        target_sources(${LIB_NAME} PRIVATE
            ${CLIENT_LIB_SRCS}
        )
    endif()

    include(plugins/platform/treeland/treeland.cmake)
    target_sources(${LIB_NAME} PRIVATE
        ${treeland_SRC}
    )
else()
    target_compile_definitions(${LIB_NAME} PRIVATE DTK_DISABLE_TREELAND)
endif()

install(FILES ${PRIVATE_HEADER_FILES} DESTINATION "${PRIVATE_INCLUDE_INSTALL_DIR}")
