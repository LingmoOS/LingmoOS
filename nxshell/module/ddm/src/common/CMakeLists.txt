configure_file("Constants.h.in" "Constants.h" IMMEDIATE @ONLY)
set(PUBLIC_HEADERS
    ${CMAKE_SOURCE_DIR}/src/common/ConfigReader.h
    ${CMAKE_SOURCE_DIR}/src/common/Configuration.h
    ${CMAKE_SOURCE_DIR}/src/common/MessageHandler.h
    ${CMAKE_SOURCE_DIR}/src/common/Messages.h
    ${CMAKE_SOURCE_DIR}/src/common/SafeDataStream.h
    ${CMAKE_SOURCE_DIR}/src/common/Session.h
    ${CMAKE_SOURCE_DIR}/src/common/SignalHandler.h
    ${CMAKE_SOURCE_DIR}/src/common/SocketWriter.h
    ${CMAKE_SOURCE_DIR}/src/common/ThemeConfig.h
    ${CMAKE_SOURCE_DIR}/src/common/ThemeMetadata.h
    ${CMAKE_SOURCE_DIR}/src/common/VirtualTerminal.h
    ${CMAKE_SOURCE_DIR}/src/common/XAuth.h
    ${CMAKE_BINARY_DIR}/src/common/Constants.h
    ${CMAKE_SOURCE_DIR}/src/common/LogindDBusTypes.h
)

set(SRCS
    ${CMAKE_SOURCE_DIR}/src/common/ConfigReader.cpp
    ${CMAKE_SOURCE_DIR}/src/common/Configuration.cpp
    ${CMAKE_SOURCE_DIR}/src/common/SafeDataStream.cpp
    ${CMAKE_SOURCE_DIR}/src/common/Session.cpp
    ${CMAKE_SOURCE_DIR}/src/common/SignalHandler.cpp
    ${CMAKE_SOURCE_DIR}/src/common/SocketWriter.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ThemeConfig.cpp
    ${CMAKE_SOURCE_DIR}/src/common/ThemeMetadata.cpp
    ${CMAKE_SOURCE_DIR}/src/common/VirtualTerminal.cpp
    ${CMAKE_SOURCE_DIR}/src/common/XAuth.cpp
    ${CMAKE_SOURCE_DIR}/src/common/LogindDBusTypes.cpp
)

add_library(common SHARED ${PUBLIC_HEADERS} ${SRCS})

set_target_properties(common PROPERTIES
    VERSION ${CMAKE_PROJECT_VERSION}
    SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR}
    OUTPUT_NAME ddm-common
    EXPORT_NAME Common
)

target_link_libraries(common
    PRIVATE
        Qt${QT_MAJOR_VERSION}::DBus
        Qt${QT_MAJOR_VERSION}::Network
        Qt${QT_MAJOR_VERSION}::Qml
        ${LIBXAU_LINK_LIBRARIES}
)

target_include_directories(common INTERFACE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/common>
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/src/common>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/ddm/common>
)

target_link_directories(common INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_LIBDIR}>
)

install(
    TARGETS common
    EXPORT CommonTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include/ddm/common
)

install(FILES ${PUBLIC_HEADERS} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/ddm/common")

install(EXPORT CommonTargets
        FILE CommonTargets.cmake
        NAMESPACE DDM::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DDM
)
