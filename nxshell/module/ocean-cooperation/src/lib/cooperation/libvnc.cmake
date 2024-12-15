if(NOT TARGET libVNC)

  # Module subdirectory
  set(libvnc_src_DIR ${CMAKE_SOURCE_DIR}/android/library/droidvnc-ng/libvncserver)
  set(libvnc_build_DIR ${CMAKE_BINARY_DIR}/libvnc)

  # build configuration
  set(WITH_EXAMPLES OFF)
  set(WITH_TESTS OFF)
  set(LIBVNCSERVER_INSTALL OFF)

  # set build as static library
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared Libs" FORCE)

  # adds a source subdir with specifying a build output dir
  add_subdirectory(${libvnc_src_DIR} ${libvnc_build_DIR})
  # need to add the build dir to include dirs as well because of generated rfbconfig.h
  include_directories(
          ${libvnc_src_DIR}/include
          ${libvnc_build_DIR}/include
  )

endif()
