add_subdirectory(baloosearch)
add_subdirectory(balooshow)

if (BUILD_INDEXER_SERVICE)
  add_subdirectory(balooctl)
endif()

if(${BUILD_EXPERIMENTAL})
  add_subdirectory(experimental/baloodb)
endif()
