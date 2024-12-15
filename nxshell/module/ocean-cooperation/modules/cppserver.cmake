if(NOT TARGET cppserver)

  # Module flag, disable build test and samples
  set(CPPSERVER_MODULE Y)

  # Module subdirectory
  set(CPPSERVER_DIR "${PROJECT_SOURCE_DIR}/3rdparty/CppServer")

  # Module subdirectory
  add_subdirectory("${CPPSERVER_DIR}" cppserver)

  include_directories("${CPPSERVER_DIR}/modules/CppLogging/include")

endif()
