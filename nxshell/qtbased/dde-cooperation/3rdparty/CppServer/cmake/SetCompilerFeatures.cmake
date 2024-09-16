# Compiler features

# Choose C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS ON)

# Configure Clang compiler options
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")

  # Clang C++ compiler should use libc++
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")

endif()

# Check for MSYS2 building environment
get_filename_component(CMAKE_MAKE_NAME ${CMAKE_MAKE_PROGRAM} NAME_WE)
if (MINGW AND CMAKE_MAKE_NAME STREQUAL "make")
  set(MSYS 1)
endif()

# Configure MSYS2/MinGW-w64 compiler options
if(MINGW OR MSYS)

  # MinGW-w64 increases the number of sections that an object file can contain
  add_compile_options(-Wa,-mbig-obj)

  # MinGW-w64 use static link std library
  add_link_options(-static-libgcc -static-libstdc++)

endif()

# Configure Visual Studio compiler options
if(MSVC)

  # Increase the number of sections that an object file can contain
  add_compile_options(/bigobj)

  # Specifies both the source character set and the execution character set as UTF-8
  # https://docs.microsoft.com/en-us/cpp/build/reference/utf-8-set-source-and-executable-character-sets-to-utf-8
  add_compile_options(/utf-8)

  # Visual Studio should correctly reports __cplusplus
  # https://devblogs.microsoft.com/cppblog/msvc-now-correctly-reports-__cplusplus
  add_compile_options(/Zc:__cplusplus)

endif()

# Configure Unix compiler options
if(UNIX)
  # Configure build in pthread library
  set(CMAKE_THREAD_LIBS_INIT "-lpthread")
  set(CMAKE_HAVE_THREADS_LIBRARY 1)
  set(CMAKE_USE_WIN32_THREADS_INIT 0)
  set(CMAKE_USE_PTHREADS_INIT 1)
  set(THREADS_PREFER_PTHREAD_FLAG ON)
endif()
