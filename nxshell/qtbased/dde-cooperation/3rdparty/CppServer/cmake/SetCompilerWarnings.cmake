# Compiler warnings

# Store origin compile flags
set(CMAKE_C_FLAGS_ORIGIN ${CMAKE_C_FLAGS})
set(CMAKE_CXX_FLAGS_ORIGIN ${CMAKE_CXX_FLAGS})

# Create custom compile flags
set(CMAKE_C_FLAGS_CUSTOM ${CMAKE_C_FLAGS})
set(CMAKE_CXX_FLAGS_CUSTOM ${CMAKE_CXX_FLAGS})

if(MSVC)

  # Set warnings level 4
  set(CMAKE_C_WARNING_LEVEL 4)
  set(CMAKE_CXX_WARNING_LEVEL 4)
  if(CMAKE_C_FLAGS_CUSTOM MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_C_FLAGS_CUSTOM ${CMAKE_C_FLAGS_CUSTOM})
  else()
    set(CMAKE_C_FLAGS_CUSTOM "${CMAKE_C_FLAGS_CUSTOM} /W4")
  endif()
  if(CMAKE_CXX_FLAGS_CUSTOM MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS_CUSTOM ${CMAKE_CXX_FLAGS_CUSTOM})
  else()
    set(CMAKE_CXX_FLAGS_CUSTOM "${CMAKE_CXX_FLAGS_CUSTOM} /W4")
  endif()

  # Make all warnings into errors
  set(CMAKE_C_FLAGS_CUSTOM "${CMAKE_C_FLAGS_CUSTOM} /WX")
  set(CMAKE_CXX_FLAGS_CUSTOM "${CMAKE_CXX_FLAGS_CUSTOM} /WX")

  # Common compile flags
  # C4100: 'identifier' : unreferenced formal parameter
  # C4250: 'class1' : inherits 'class2::member' via dominance
  set(COMMON_COMPILE_FLAGS "/wd4100 /wd4250")

  # Pedantic compile flags
  set(PEDANTIC_COMPILE_FLAGS ${COMMON_COMPILE_FLAGS})

else()

  # Make all warnings into errors
  set(CMAKE_C_FLAGS_CUSTOM "${CMAKE_C_FLAGS_CUSTOM} -Wall -Werror")
  set(CMAKE_CXX_FLAGS_CUSTOM "${CMAKE_CXX_FLAGS_CUSTOM} -Wall -Werror")

  # Common compile flags
  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(COMMON_COMPILE_FLAGS "-Wno-maybe-uninitialized -Wno-overflow -Wno-restrict -Wno-stringop-overflow")
  else()
    set(COMMON_COMPILE_FLAGS "")
  endif()

  # Pedantic compile flags
  set(PEDANTIC_COMPILE_FLAGS "${COMMON_COMPILE_FLAGS} -Wshadow -pedantic")

endif()

# Update compile flags
set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS_CUSTOM})
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS_CUSTOM})
