include(CheckTypeSize)
include(CheckIncludeFile)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(CMAKE_EXTRA_INCLUDE_FILES "linux/uinput.h")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    set(CMAKE_EXTRA_INCLUDE_FILES "dev/evdev/uinput.h")
endif()

check_type_size("struct uinput_setup" STRUCT_UINPUT_SETUP)
set(CMAKE_EXTRA_INCLUDE_FILES)

if(HAVE_STRUCT_UINPUT_SETUP)
    add_executable(faketablet faketablet.cpp)
else()
    message(STATUS "No suitable uinput API, fake tablet is disabled")
endif()

add_executable(setmapping setmapping.cpp)
target_link_libraries(setmapping ${WACOM_KDED_TEST_LIBS})
