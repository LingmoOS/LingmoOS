
add_subdirectory(api)
add_subdirectory(runtime)

ecm_qt_install_logging_categories(
    EXPORT KWALLET
    FILE kwallet.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)
