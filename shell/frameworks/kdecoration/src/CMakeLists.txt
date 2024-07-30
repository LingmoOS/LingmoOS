add_definitions(-DTRANSLATION_DOMAIN=\"kdecoration\")

add_subdirectory(private)

set(libkdecoration2_SRCS
    decoratedclient.cpp
    decoratedclient.h
    decorationbutton.cpp
    decorationbuttongroup.cpp
    decorationbuttongroup.h
    decorationbuttongroup_p.h
    decorationbutton.h
    decorationbutton_p.h
    decoration.cpp
    decorationdefines.h
    decoration.h
    decoration_p.h
    decorationsettings.cpp
    decorationsettings.h
    decorationshadow.cpp
    decorationshadow.h
    decorationshadow_p.h
    decorationthemeprovider.cpp
    decorationthemeprovider.h

)

add_library(kdecorations2 SHARED ${libkdecoration2_SRCS})
ecm_generate_export_header(kdecorations2
    VERSION ${PROJECT_VERSION}
    EXPORT_FILE_NAME kdecoration2/kdecoration2_export.h
    DEPRECATION_VERSIONS 5.21
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

add_library(KDecoration2::KDecoration ALIAS kdecorations2)

target_link_libraries(kdecorations2
    PUBLIC
        Qt::Core
        Qt::Gui
    PRIVATE
        kdecorations2private
        KF6::I18n
)

target_include_directories(kdecorations2 INTERFACE "$<INSTALL_INTERFACE:${KDECORATION2_INCLUDEDIR}>" )

set_target_properties(kdecorations2 PROPERTIES VERSION   ${KDECORATION2_VERSION}
                                               SOVERSION ${KDECORATION2_SOVERSION}
                                               EXPORT_NAME KDecoration
)

ecm_generate_headers(KDecoration2_CamelCase_HEADERS
  HEADER_NAMES
    DecoratedClient
    Decoration
    DecorationButton
    DecorationButtonGroup
    DecorationSettings
    DecorationShadow
    DecorationThemeProvider
  PREFIX
    KDecoration2
  REQUIRED_HEADERS KDecoration2_HEADERS
)
install(FILES ${KDecoration2_CamelCase_HEADERS}
        DESTINATION ${KDECORATION2_INCLUDEDIR}/KDecoration2
        COMPONENT Devel)

install(TARGETS kdecorations2 EXPORT KDecoration2Targets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/kdecoration2/kdecoration2_export.h
        ${KDecoration2_HEADERS}
        decorationdefines.h
    DESTINATION
        ${KDECORATION2_INCLUDEDIR}/kdecoration2
    COMPONENT
        Devel
)
