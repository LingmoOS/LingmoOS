get_filename_component(generated_name ${generated} NAME)

if(NOT ${generated_name} STREQUAL "testpackage.appdata.xml")
    message(STATUS "cd ${input} ; ${kpackagetool} --appstream-metainfo . --appstream-metainfo-output ${generated}")
    execute_process(COMMAND ${kpackagetool} --appstream-metainfo . --appstream-metainfo-output "${generated}"
        WORKING_DIRECTORY ${input}
        ERROR_VARIABLE std_error_output)
else()
    message(STATUS "cd ${input} ; ${kpackagetool} --appstream-metainfo .")
    # Make sure that without output argument we get the output on STDOUT.
    # This only runs on the testpackage test.
    execute_process(COMMAND ${kpackagetool} --appstream-metainfo .
        WORKING_DIRECTORY ${input}
        OUTPUT_FILE "${generated}"
        ERROR_VARIABLE std_error_output)
endif()

if (std_error_output)
    message(STATUS "${std_error_output}")
endif()

# We'll want no generated output file at all on NoDisplay. For other tests we'll want to compare
# the expected with the generated output.
if(${generated_name} STREQUAL "testpackage-nodisplay.appdata.xml")
    if(EXISTS "${generated}")
        message(FATAL_ERROR "expected file ${output} does not exist, but one was generated at ${generated}")
    endif()
else()
    execute_process(COMMAND cmake -E compare_files ${output} ${generated} ERROR_VARIABLE error_compare)
    if (error_compare)
        message(FATAL_ERROR "error on compare: ${error_compare}")
    endif()
endif()

# Make sure the standard test passes appstream validation.
if(${generated_name} STREQUAL "testpackage.appdata.xml")
    find_program(APPSTREAMCLI appstreamcli)
    if(APPSTREAMCLI)
        execute_process(COMMAND ${APPSTREAMCLI} validate ${generated}
            ERROR_VARIABLE appstream_stderr
            OUTPUT_VARIABLE appstream_stdout
            RESULT_VARIABLE result
        )
        if(NOT result EQUAL 0)
            message(FATAL_ERROR "appstream data seems to be imperfect: ${appstream_stderr} ${appstream_stdout}")
        endif()
    else()
        message(WARNING "skipping appstream validation as no appstreamcli binary was found")
    endif()
endif()
