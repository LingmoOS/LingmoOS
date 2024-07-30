add_library(oceancommon${QT_MAJOR_VERSION} OBJECT oceanboxshadowrenderer.cpp)

set_target_properties(oceancommon${QT_MAJOR_VERSION} PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

target_link_libraries(oceancommon${QT_MAJOR_VERSION}
    PUBLIC
        Qt${QT_MAJOR_VERSION}::Core
        Qt${QT_MAJOR_VERSION}::Gui)
