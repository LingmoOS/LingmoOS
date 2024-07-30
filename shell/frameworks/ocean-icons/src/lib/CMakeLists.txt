# helper functions to ensure we load the icon theme
set(koceanicons_SRCS oceanicons.cpp)

# avoid dependency cycle with add resource
set(CMAKE_AUTOMOC OFF)

# generate resource file for all ocean icons
set(RESOURCE_FILE ${CMAKE_CURRENT_BINARY_DIR}/ocean-icons.qrc)
set(RESOURCE_FILE_BINARY ${CMAKE_CURRENT_BINARY_DIR}/ocean-icons.rcc)

# we only will use the normal icons, we do recoloring later
# we run the tool over the dark ones just to do all checks on them, too
add_custom_command(OUTPUT ${RESOURCE_FILE}
    COMMAND qrcAlias -o ${RESOURCE_FILE} ${CMAKE_SOURCE_DIR}/icons ${CMAKE_BINARY_DIR}/ocean-icons # used for the resource
    COMMAND qrcAlias -o ${RESOURCE_FILE}.dark.dummy ${CMAKE_SOURCE_DIR}/icons-dark ${CMAKE_BINARY_DIR}/ocean-icons-dark # used for checking only
    COMMAND Qt6::rcc --binary -o ${RESOURCE_FILE_BINARY} ${RESOURCE_FILE} # FIXME: remove after e.g. Kexi is ported
    DEPENDS qrcAlias ocean-generate-24px-versions ocean-generate-24px-versions-dark # ensure we run after the generation steps
    USES_TERMINAL
)

# install as compatibility resource file if really requested
# FIXME: remove after e.g. Kexi is ported
if(BINARY_ICONS_RESOURCE)
    install(FILES ${RESOURCE_FILE_BINARY} DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/ocean)
endif()

# use normal resource adding, big resource variant has issue with LTO
qt_add_resources(koceanicons_resource_SRCS
    ${RESOURCE_FILE}
    OPTIONS --root /icons/ocean
)

add_library(KF6OceanIcons ${koceanicons_resource_SRCS} ${koceanicons_SRCS})
add_library(KF6::OceanIcons ALIAS KF6OceanIcons)

set_target_properties(KF6OceanIcons
    PROPERTIES
        VERSION ${OCEANICONS_VERSION}
        SOVERSION ${OCEANICONS_SOVERSION}
        EXPORT_NAME OceanIcons
)

generate_export_header(KF6OceanIcons BASE_NAME OceanIcons)

target_include_directories(KF6OceanIcons
    INTERFACE
        "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/OceanIcons>"
)

target_link_libraries(KF6OceanIcons
    PUBLIC
        Qt6::Gui
)

ecm_generate_headers(KF6OceanIcons_CamelCase_HEADERS
    HEADER_NAMES
        OceanIcons

    REQUIRED_HEADERS
        KF6OceanIcons_HEADERS
)

install(TARGETS KF6OceanIcons EXPORT KF6OceanIconsTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/oceanicons_export.h
    ${KF6OceanIcons_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/OceanIcons
    COMPONENT Devel
)

install(FILES
    ${KF6OceanIcons_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/OceanIcons
    COMPONENT Devel
)

