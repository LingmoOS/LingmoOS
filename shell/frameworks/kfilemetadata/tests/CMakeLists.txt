remove_definitions(-DQT_NO_CAST_FROM_ASCII)

add_executable(dump dump.cpp)

target_include_directories(dump  PUBLIC "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>")

target_link_libraries(dump
    Qt6::Core
    KF6FileMetaData
)

# Though the dump utility is not a regular test case, running it increases
# code coverage significantly
# We check if the output matches using a regular expression
ecm_mark_as_test(dump)
ecm_mark_nongui_executable(dump)

add_test(NAME dump_fail COMMAND dump "${CMAKE_CURRENT_SOURCE_DIR}")
set_tests_properties(dump_fail PROPERTIES
    # Directories are not valid arguments
    WILL_FAIL TRUE
)

add_test(NAME dump_meta COMMAND dump "${CMAKE_CURRENT_SOURCE_DIR}/../autotests/samplefiles/test_plain_text_file.txt")
set_tests_properties(dump_meta PROPERTIES
    PASS_REGULAR_EXPRESSION "PlaintextExtractor"
)

add_test(NAME dump_fulltext COMMAND dump "-f" "${CMAKE_CURRENT_SOURCE_DIR}/../autotests/samplefiles/test_plain_text_file.txt")
set_tests_properties(dump_fulltext PROPERTIES
    PASS_REGULAR_EXPRESSION "Text: This"
)

add_test(NAME dump_meta_svg COMMAND dump "${CMAKE_CURRENT_SOURCE_DIR}/../autotests/samplefiles/test_dcterms.svg")
set_tests_properties(dump_meta_svg PROPERTIES
    PASS_REGULAR_EXPRESSION "Description: A test document"
)
