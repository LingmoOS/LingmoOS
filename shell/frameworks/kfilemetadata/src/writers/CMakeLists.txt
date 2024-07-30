if(Taglib_FOUND)
    add_library(kfilemetadata_taglibwriter MODULE taglibwriter.cpp ${debug_SRCS})

    target_link_libraries( kfilemetadata_taglibwriter
        KF6::FileMetaData
        Taglib::Taglib
    )

    set_target_properties(kfilemetadata_taglibwriter PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/kf6/kfilemetadata/writers")
    install(
    TARGETS kfilemetadata_taglibwriter
    DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kfilemetadata/writers)

endif()
