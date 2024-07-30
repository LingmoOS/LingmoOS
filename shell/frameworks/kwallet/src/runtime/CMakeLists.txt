if(BUILD_KWALLETD)
    add_subdirectory(kwalletd)
endif()

if(BUILD_KWALLET_QUERY)
  add_subdirectory(kwallet-query)
endif()
