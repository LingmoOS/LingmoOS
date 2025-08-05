set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

aux_source_directory(. build_grammar_src)
include_directories(../src)

# msvc doesn't export all symbols
if(NOT (WIN32 AND BUILD_SHARED_LIBS))
  add_executable(build_grammar
    ${build_grammar_src}
    $<TARGET_OBJECTS:rime-octagram-objs>)
  target_link_libraries(build_grammar
    ${rime_library}
    ${rime_dict_library})
endif()
