add_subdirectory(desktoptheme)
add_subdirectory(lingmo)
add_subdirectory(declarativeimports)
add_subdirectory(lingmoquick)

ecm_qt_install_logging_categories(
    EXPORT LINGMO
    FILE lingmo-framework.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)
