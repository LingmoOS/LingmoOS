
add_library(LingmoUIPrimitives)
ecm_add_qml_module(LingmoUIPrimitives URI "org.kde.lingmoui.primitives"
    VERSION 2.0
    GENERATE_PLUGIN_SOURCE
    DEPENDENCIES QtQuick org.kde.lingmoui.platform
)

target_sources(LingmoUIPrimitives PRIVATE
    icon.cpp
    icon.h
    shadowedrectangle.cpp
    shadowedrectangle.h
    shadowedtexture.cpp
    shadowedtexture.h

    scenegraph/managedtexturenode.cpp
    scenegraph/managedtexturenode.h
    scenegraph/paintedrectangleitem.cpp
    scenegraph/paintedrectangleitem.h
    scenegraph/shadowedborderrectanglematerial.cpp
    scenegraph/shadowedborderrectanglematerial.h
    scenegraph/shadowedbordertexturematerial.cpp
    scenegraph/shadowedbordertexturematerial.h
    scenegraph/shadowedrectanglematerial.cpp
    scenegraph/shadowedrectanglematerial.h
    scenegraph/shadowedrectanglenode.cpp
    scenegraph/shadowedrectanglenode.h
    scenegraph/shadowedtexturematerial.cpp
    scenegraph/shadowedtexturematerial.h
    scenegraph/shadowedtexturenode.cpp
    scenegraph/shadowedtexturenode.h
)

ecm_target_qml_sources(LingmoUIPrimitives SOURCES
    Separator.qml
    ShadowedImage.qml
)

set_target_properties(LingmoUIPrimitives PROPERTIES
    VERSION     ${PROJECT_VERSION}
    SOVERSION   6
    EXPORT_NAME "LingmoUIPrimitives"
)

target_include_directories(LingmoUIPrimitives PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(LingmoUIPrimitives PRIVATE Qt6::Quick LingmoUIPlatform)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    set(_extra_options DEBUGINFO)
else()
    set(_extra_options PRECOMPILE OPTIMIZED)
endif()

qt6_add_shaders(LingmoUIPrimitives "shaders"
    BATCHABLE
    PREFIX "/qt/qml/org/kde/lingmoui/primitives/shaders"
    FILES
        shaders/shadowedrectangle.vert
        shaders/shadowedrectangle.frag
        shaders/shadowedrectangle_lowpower.frag
        shaders/shadowedborderrectangle.frag
        shaders/shadowedborderrectangle_lowpower.frag
        shaders/shadowedtexture.frag
        shaders/shadowedtexture_lowpower.frag
        shaders/shadowedbordertexture.frag
        shaders/shadowedbordertexture_lowpower.frag
    OUTPUTS
        shadowedrectangle.vert.qsb
        shadowedrectangle.frag.qsb
        shadowedrectangle_lowpower.frag.qsb
        shadowedborderrectangle.frag.qsb
        shadowedborderrectangle_lowpower.frag.qsb
        shadowedtexture.frag.qsb
        shadowedtexture_lowpower.frag.qsb
        shadowedbordertexture.frag.qsb
        shadowedbordertexture_lowpower.frag.qsb
    ${_extra_options}
)

include_static_dependencies(LingmoUIPrimitives EXPORT LingmoUITargets)

ecm_finalize_qml_module(LingmoUIPrimitives)

if (NOT BUILD_SHARED_LIBS)
    install(TARGETS LingmoUIPrimitivesplugin EXPORT LingmoUITargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
endif()

install(TARGETS LingmoUIPrimitives EXPORT LingmoUITargets ${KF_INSTALL_DEFAULT_ARGUMENTS})
