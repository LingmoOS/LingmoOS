find_file(QDOC_TEMPLATE global/qt-html-templates-offline.qdocconf
    HINTS ${QT6_INSTALL_PREFIX}/${QT6_INSTALL_DOCS}
)

get_filename_component(QDOC_TEMPLATE_DIR ${QDOC_TEMPLATE} DIRECTORY)
if(NOT QDOC_TEMPLATE_DIR)
    message(STATUS "Could not build .qch because the qdoc template directory could not be found")
endif()

function(_qdoc_status_report)
    message(STATUS "Unable to build .qch user manual:")

    if(TARGET Qt6::qdoc)
        get_property(_path TARGET Qt6::qdoc PROPERTY LOCATION)
        message(STATUS "qdoc executable: ${_path}")
    else()
        message(STATUS "qdoc executable not found")
    endif()

    if(TARGET Qt6::qhelpgenerator)
        get_property(_path TARGET Qt6::qhelpgenerator PROPERTY LOCATION)
        message(STATUS "qhelpgenerator executable: ${_path}")
    else()
        message(STATUS "qhelpgenerator not found")
    endif()
endfunction()

set(_qdoc_available FALSE)
if(QDOC_TEMPLATE_DIR AND TARGET Qt6::qdoc AND TARGET Qt6::qhelpgenerator)
    set(_qdoc_available TRUE)
endif()

# If qdoc and qhelpgenerator are found build .qch
if(NOT _qdoc_available)
    _qdoc_status_report()
else()
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/user-feedback-manual.qdocconf.in ${CMAKE_CURRENT_BINARY_DIR}/user-feedback-manual.qdocconf)
    file(GLOB_RECURSE _qdoc_srcs ${CMAKE_CURRENT_SOURCE_DIR} "*.qdoc")
    set(_qdoc_output_dir ${CMAKE_CURRENT_BINARY_DIR}/manual)
    add_custom_command(
        OUTPUT ${_qdoc_output_dir}/user-feedback-manual.qhp
        COMMAND Qt6::qdoc
            --outputdir ${_qdoc_output_dir}
            ${CMAKE_CURRENT_BINARY_DIR}/user-feedback-manual.qdocconf
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/user-feedback-manual.qdocconf ${_qdoc_srcs}
    )
    add_custom_command(
        OUTPUT ${_qdoc_output_dir}/user-feedback-manual.qch
        COMMAND Qt6::qhelpgenerator ${_qdoc_output_dir}/user-feedback-manual.qhp
        DEPENDS ${_qdoc_output_dir}/user-feedback-manual.qhp
    )
    add_custom_target(user_feedback_manual_qch ALL DEPENDS ${_qdoc_output_dir}/user-feedback-manual.qch)
    install(FILES ${_qdoc_output_dir}/user-feedback-manual.qch DESTINATION ${KDE_INSTALL_DATAROOTDIR}/KDE/UserFeedbackConsole)
endif()

# If qcollectiongenerator is found build .qhc
if(NOT TARGET Qt6::qcollectiongenerator OR NOT _qdoc_available)
    message(STATUS "Unable to build .qhc user manual:")
    if(TARGET Qt6::qcollectiongenerator)
        get_property(_path TARGET Qt6::qcollectiongenerator PROPERTY LOCATION)
        message(STATUS "qcollectiongenerator executable: ${_path}")
    else()
        message(STATUS "qcollectiongenerator not found")
    endif()

    _qdoc_status_report()
else()
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/user-feedback.qhcp.in ${CMAKE_CURRENT_BINARY_DIR}/user-feedback.qhcp)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/user-feedback.qhc
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/manual/user-feedback-manual.qch ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND Qt6::qcollectiongenerator ${CMAKE_CURRENT_BINARY_DIR}/user-feedback.qhcp -o ${CMAKE_CURRENT_BINARY_DIR}/user-feedback.qhc
        DEPENDS
            ${CMAKE_CURRENT_BINARY_DIR}/user-feedback.qhcp
            user_feedback_manual_qch
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    add_custom_target(user_feedback_qhc ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/user-feedback.qhc)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/user-feedback.qhc DESTINATION ${KDE_INSTALL_DATAROOTDIR}/KDE/UserFeedbackConsole)
endif()
