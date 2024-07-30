cmake_minimum_required(VERSION 3.16)
project(KApiDox NONE)

set(KF_VERSION "6.5.0") # handled by release scripts

if(DO_BOOTSTRAP)
    include(FindUnixCommands)

    if(WIN32)
        # Needs native path and extra escaping of spaces
        file(TO_NATIVE_PATH "${CMAKE_BINARY_DIR}" BINARY_DIR)
    else()
        set(BINARY_DIR ${CMAKE_BINARY_DIR})
    endif()

    add_custom_target(build ALL
        COMMAND ${BASH} bootstrap-devenv.sh
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
endif()

# A dummy install part to fulfill needs of kdesrc-build and possible make installs
install(RUNTIME_DEPENDENCY_SET kapidox OPTIONAL)
