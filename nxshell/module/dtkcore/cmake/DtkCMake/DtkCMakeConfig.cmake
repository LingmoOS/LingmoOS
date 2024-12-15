function(addDefinitions macro)
    string(TOUPPER ${macro} macro)
    add_definitions(-D${macro})
endfunction()

add_definitions(-DQ_HOST_NAME=\"${CMAKE_HOST_SYSTEM_PROCESSOR}\")
addDefinitions(Q_HOST_${CMAKE_HOST_SYSTEM_PROCESSOR})

find_package(DtkCore REQUIRED)

set(LINGMO_OS_RELEASE_TOOL_PATH ${DtkCore_TOOL_DIRS})
set(LINGMO_OS_RELEASE_TOOL ${LINGMO_OS_RELEASE_TOOL_PATH}/lingmo-os-release)

if(NOT EXISTS "${LINGMO_OS_RELEASE_TOOL}")
    message(FATAL_ERROR "\"${LINGMO_OS_RELEASE_TOOL}\" is not exists. Install \"dtkcore-bin\" first")
endif()

function(formatString string)
    string(REGEX REPLACE "\\s+" "_" string ${string})
endfunction()

macro(execLingmoOsRelease args output)
    exec_program(${LINGMO_OS_RELEASE_TOOL} ARGS ${args} OUTPUT_VARIABLE ${output} RETURN_VALUE exitCode)

    if(NOT ${exitCode} EQUAL 0)
        message(FATAL_ERROR "exec lingmo-os-release failed, with args: ${args}, error message: ${output}")
    endif()
endmacro()

execLingmoOsRelease(--lingmo-type LINGMO_OS_TYPE)
execLingmoOsRelease(--lingmo-version LINGMO_OS_VERSION)
execLingmoOsRelease(--product-type CMAKE_PLATFORM_ID)
execLingmoOsRelease(--product-version CMAKE_PLATFORM_VERSION)

if("${CMAKE_PLATFORM_ID}" STREQUAL "")
    message(WARNING "No value of the \"--product-type\" in the process \"${LINGMO_OS_RELEASE_TOOL}\"")
else()
    formatString(CMAKE_PLATFORM_ID)

    message("OS: ${CMAKE_PLATFORM_ID}, Version: ${CMAKE_PLATFORM_VERSION}")

    if(NOT "${CMAKE_PLATFORM_ID}" STREQUAL "")
        addDefinitions(Q_OS_${CMAKE_PLATFORM_ID})
        string(TOUPPER ${CMAKE_PLATFORM_ID} CMAKE_PLATFORM_ID)
        set(OS_${CMAKE_PLATFORM_ID} TRUE)
    endif()

    formatString(CMAKE_PLATFORM_VERSION)
    add_definitions(-DQ_OS_VERSION=\"${CMAKE_PLATFORM_VERSION}\")

    #uos base with lingmo
    if("${CMAKE_PLATFORM_ID}" STREQUAL "UOS")
        addDefinitions(Q_OS_LINGMO)
        set(OS_LINGMO TRUE)
    endif()
endif()

if("${LINGMO_OS_TYPE}" STREQUAL "")
    message(WARNING "No value of the \"--lingmo-type\" in the process \"${LINGMO_OS_RELEASE_TOOL}\"")
else()
    formatString(LINGMO_OS_TYPE)

    message("Lingmo OS Type: ${LINGMO_OS_TYPE}")
    message("Lingmo OS Version: ${LINGMO_OS_VERSION}")

    if(NOT "${LINGMO_OS_TYPE}" STREQUAL "")
        addDefinitions(Q_OS_LINGMO_${LINGMO_OS_TYPE})
        addDefinitions(LINGMO_OCEAN)
        string(TOUPPER ${LINGMO_OS_TYPE} LINGMO_OS_TYPE)
        set(OS_LINGMO_${LINGMO_OS_TYPE} TRUE)
        set(LINGMO_OCEAN TRUE)
    endif()

    formatString(LINGMO_OS_VERSION)
    add_definitions(-DQ_OS_LINGMO_VERSION=\"${LINGMO_OS_VERSION}\")
endif()
