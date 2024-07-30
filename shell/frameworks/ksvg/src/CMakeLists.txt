add_subdirectory(ksvg)
add_subdirectory(declarativeimports)

if (BUILD_TOOLS)
    add_subdirectory(tools)
endif()

ecm_qt_install_logging_categories(
    EXPORT KSVG
    FILE ksvg.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)
