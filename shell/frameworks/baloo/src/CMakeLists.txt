add_subdirectory(lib)
add_subdirectory(engine)
add_subdirectory(codecs)
add_subdirectory(qml)

if (BUILD_KINOTIFY)
    add_subdirectory(file)
    add_subdirectory(kioworkers)
    add_subdirectory(tools)
endif()

add_subdirectory(dbus)

ecm_qt_install_logging_categories(
    EXPORT BALOO
    FILE baloo.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)
