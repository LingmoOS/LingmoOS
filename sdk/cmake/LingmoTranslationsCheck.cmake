# Checks every catalog of PO_FILES with msgfmt: syntax, plural forms, header
# and the "{name}" placeholders; see LingmoTranslations.cmake.
string(REPLACE "|" ";" PO_FILES "${PO_FILES}")

set(failed FALSE)
foreach(po_file ${PO_FILES})
    execute_process(
        COMMAND ${MSGFMT} --check --statistics -o /dev/null ${po_file}
        RESULT_VARIABLE result
        ERROR_VARIABLE output)
    get_filename_component(name ${po_file} NAME)
    string(STRIP "${output}" output)
    message(STATUS "${name}: ${output}")
    if(NOT result EQUAL 0)
        set(failed TRUE)
    endif()
endforeach()
if(failed)
    message(FATAL_ERROR "Some catalogs have errors")
endif()
