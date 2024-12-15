# Platform features

if(CYGWIN)

  # Base Windows platform
  add_definitions(-DWIN32 -DWIN32_LEAN_AND_MEAN)

  # Windows 10
  add_definitions(-D_WIN32_WINNT=0x0A00)

elseif(WIN32)

  # Base Windows platform
  add_definitions(-DWIN32 -DWIN32_LEAN_AND_MEAN)

  # Disable CRT secure warnings
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE)

  # Disable C++17 deprecation warnings
  add_definitions(-D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)

  # Windows 7
  add_definitions(-D_WIN32_WINNT=0x0601)

  # Windows SDK
  if(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION AND NOT MINGW)
    string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)\\.([0-9]+)$" "\\3" CMAKE_WIN32_SDK ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION})
  else()
    string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)$" "\\3" CMAKE_WIN32_SDK ${CMAKE_SYSTEM_VERSION})
  endif()
  add_definitions(-D_WIN32_SDK=${CMAKE_WIN32_SDK})

endif()
