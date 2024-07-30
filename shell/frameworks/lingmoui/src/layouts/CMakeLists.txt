
add_library(LingmoUILayouts)
ecm_add_qml_module(LingmoUILayouts URI "org.kde.lingmoui.layouts"
    VERSION 2.0
    GENERATE_PLUGIN_SOURCE
    DEPENDENCIES QtQuick org.kde.lingmoui.platform
)

ecm_qt_declare_logging_category(LingmoUILayouts
    HEADER loggingcategory.h
    IDENTIFIER LingmoUILayoutsLog
    CATEGORY_NAME kf.lingmoui.layouts
    DESCRIPTION "LingmoUILayouts"
    DEFAULT_SEVERITY Warning
    EXPORT LINGMOUI
)

target_sources(LingmoUILayouts PRIVATE
    columnview.cpp
    displayhint.cpp
    formlayoutattached.cpp
    headerfooterlayout.cpp
    padding.cpp
    sizegroup.cpp
    toolbarlayout.cpp
    toolbarlayoutdelegate.cpp
)

ecm_target_qml_sources(LingmoUILayouts SOURCES
    FormLayout.qml
)

set_target_properties(LingmoUILayouts PROPERTIES
    VERSION     ${PROJECT_VERSION}
    SOVERSION   6
    EXPORT_NAME "LingmoUILayouts"
)

target_include_directories(LingmoUILayouts PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(LingmoUILayouts PRIVATE Qt6::Quick LingmoUIPlatform)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    set(_extra_options DEBUGINFO)
else()
    set(_extra_options PRECOMPILE OPTIMIZED)
endif()

include_static_dependencies(LingmoUILayouts EXPORT LingmoUITargets)

ecm_finalize_qml_module(LingmoUILayouts)

if (NOT BUILD_SHARED_LIBS)
    install(TARGETS LingmoUILayoutsplugin EXPORT LingmoUITargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
endif()

install(TARGETS LingmoUILayouts EXPORT LingmoUITargets ${KF_INSTALL_DEFAULT_ARGUMENTS})
