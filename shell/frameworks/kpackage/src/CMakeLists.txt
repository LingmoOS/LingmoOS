add_subdirectory(kpackage)
add_subdirectory(kpackagetool)

ecm_qt_install_logging_categories(
    EXPORT KPACKAGE
    FILES kpackage.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)
