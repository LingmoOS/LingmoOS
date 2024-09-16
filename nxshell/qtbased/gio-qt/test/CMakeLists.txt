find_package(Qt6 REQUIRED COMPONENTS Test)

enable_testing ()

function(DGIO_CREATE_TEST _generated_target_name _input_file_name _use_giomm)
    set (extra_libraries)
    if (_use_giomm)
        list(APPEND extra_libraries ${GIOMM_LIBRARIES})
    endif ()

    add_executable (${_generated_target_name}
        ${_input_file_name}
    )
    add_test (NAME ${_generated_target_name} COMMAND ${_generated_target_name} )
    target_link_libraries (${_generated_target_name} gio-qt Qt5::Test ${extra_libraries})
    if (_use_giomm)
        target_include_directories(${_generated_target_name} PRIVATE ${GIOMM_INCLUDE_DIRS})
        target_compile_definitions(${_generated_target_name} PRIVATE QT_NO_KEYWORDS)
    endif ()
endfunction()


# Match GIO Enum
dgio_create_test (tst_matchgioenum tst_matchgioenum.cpp YES)

# Simple FileInfo
dgio_create_test (tst_simplefileinfo tst_simplefileinfo.cpp NO)

dgio_create_test (tst_dgiosettings tst_dgiosettings.cpp NO)
