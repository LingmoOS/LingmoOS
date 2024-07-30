add_subdirectory(lib)
add_subdirectory(tools)
if(TARGET Qt6::Quick)
    add_subdirectory(quick)
endif()
if(TARGET Qt6::Multimedia AND TARGET ZXing::ZXing)
    add_subdirectory(scanner)
    if (TARGET Qt6::Quick)
        add_subdirectory(scanner-quick)
    endif()
endif()

ecm_qt_install_logging_categories(
    EXPORT PRISON
    FILE prison.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)
