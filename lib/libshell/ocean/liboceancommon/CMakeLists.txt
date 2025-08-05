################# dependencies #################
### Qt/KDE
find_package(Qt5 REQUIRED CONFIG COMPONENTS Widgets)

################# oceanstyle target #################
set(oceancommon_LIB_SRCS
    oceanboxshadowrenderer.cpp
)

add_library(oceancommon5 ${oceancommon_LIB_SRCS})

generate_export_header(oceancommon5
    BASE_NAME oceancommon
    EXPORT_FILE_NAME oceancommon_export.h)

target_link_libraries(oceancommon5
    PUBLIC
        Qt5::Core
        Qt5::Gui)

set_target_properties(oceancommon5 PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR})

install(TARGETS oceancommon5 ${INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)
