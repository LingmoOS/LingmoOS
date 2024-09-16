if(NOT TARGET sslconf)

  # Module subdirectory
  add_subdirectory(sslconf)

  # Module folder
  set_target_properties(sslconf PROPERTIES FOLDER "modules/sslconf")

endif()
