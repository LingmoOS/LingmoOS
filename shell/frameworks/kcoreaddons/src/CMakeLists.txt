add_subdirectory(lib)
add_subdirectory(mimetypes)
if (KCOREADDONS_USE_QML)
    add_subdirectory(qml)
endif()

ecm_qt_install_logging_categories(
    EXPORT KCOREADDONS
    FILE kcoreaddons.categories
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)
