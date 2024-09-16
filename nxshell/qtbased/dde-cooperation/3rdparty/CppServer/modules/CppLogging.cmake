if(NOT TARGET cpplogging)

  # Module flag
  set(CPPLOGGING_MODULE Y)

  # Module subdirectory
  add_subdirectory("CppLogging")

  # Module folder
  set_target_properties(cpplogging PROPERTIES FOLDER "modules/CppLogging")

endif()
