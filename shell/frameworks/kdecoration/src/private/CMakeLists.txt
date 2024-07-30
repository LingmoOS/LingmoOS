set(libkdecoration2Private_SRCS
    decoratedclientprivate.cpp
    decoratedclientprivate.h
    decorationbridge.cpp
    decorationbridge.h
    decorationsettingsprivate.cpp
    decorationsettingsprivate.h
)

add_library(kdecorations2private SHARED ${libkdecoration2Private_SRCS})

generate_export_header(
    kdecorations2private
BASE_NAME
    KDECORATIONS_PRIVATE
EXPORT_FILE_NAME
    kdecoration2/private/kdecoration2_private_export.h
)

add_library(KDecoration2::KDecorationPrivate ALIAS kdecorations2private)

target_link_libraries(kdecorations2private
    PUBLIC
        Qt::Core
        Qt::Gui
)

target_include_directories(kdecorations2private INTERFACE "$<INSTALL_INTERFACE:${KDECORATION2_INCLUDEDIR}>" )

set_target_properties(kdecorations2private PROPERTIES VERSION   ${KDECORATION2_VERSION}
                                                      SOVERSION 11
                                                      EXPORT_NAME KDecoration2Private
)

ecm_generate_headers(KDecoration2Private_CamelCase_HEADERS
  HEADER_NAMES
    DecoratedClientPrivate
    DecorationBridge
    DecorationSettingsPrivate
  PREFIX
    KDecoration2/Private
  REQUIRED_HEADERS KDecoration2Private_HEADERS
)
install(FILES ${KDecoration2Private_CamelCase_HEADERS}
        DESTINATION ${KDECORATION2_INCLUDEDIR}/KDecoration2/Private
        COMPONENT Devel)

install(TARGETS kdecorations2private EXPORT KDecoration2Targets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/kdecoration2/private/kdecoration2_private_export.h
        ${KDecoration2Private_HEADERS}
    DESTINATION
        ${KDECORATION2_INCLUDEDIR}/kdecoration2/private
    COMPONENT
        Devel
)
