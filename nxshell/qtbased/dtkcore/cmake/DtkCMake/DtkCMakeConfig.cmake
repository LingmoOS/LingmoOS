function(addDefinitions macro)
    string(TOUPPER ${macro} macro)
    add_definitions(-D${macro})
endfunction()

add_definitions(-DQ_HOST_NAME=\"${CMAKE_HOST_SYSTEM_PROCESSOR}\")
addDefinitions(Q_HOST_${CMAKE_HOST_SYSTEM_PROCESSOR})

find_package(DtkCore REQUIRED)

set(DEEPIN_OS_RELEASE_TOOL_PATH ${DtkCore_TOOL_DIRS})
set(DEEPIN_OS_RELEASE_TOOL ${DEEPIN_OS_RELEASE_TOOL_PATH}/deepin-os-release)

if(NOT EXISTS "${DEEPIN_OS_RELEASE_TOOL}")
    message(FATAL_ERROR "\"${DEEPIN_OS_RELEASE_TOOL}\" is not exists. Install \"dtkcore-bin\" first")
endif()

function(formatString string)
    string(REGEX REPLACE "\\s+" "_" string ${string})
endfunction()

macro(execDeepinOsRelease args output)
    exec_program(${DEEPIN_OS_RELEASE_TOOL} ARGS ${args} OUTPUT_VARIABLE ${output} RETURN_VALUE exitCode)

    if(NOT ${exitCode} EQUAL 0)
        message(FATAL_ERROR "exec deepin-os-release failed, with args: ${args}, error message: ${output}")
    endif()
endmacro()

execDeepinOsRelease(--deepin-type DEEPIN_OS_TYPE)
execDeepinOsRelease(--deepin-version DEEPIN_OS_VERSION)
execDeepinOsRelease(--product-type CMAKE_PLATFORM_ID)
execDeepinOsRelease(--product-version CMAKE_PLATFORM_VERSION)

if("${CMAKE_PLATFORM_ID}" STREQUAL "")
    message(WARNING "No value of the \"--product-type\" in the process \"${DEEPIN_OS_RELEASE_TOOL}\"")
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

    #uos base with deepin
    if("${CMAKE_PLATFORM_ID}" STREQUAL "UOS")
        addDefinitions(Q_OS_DEEPIN)
        set(OS_DEEPIN TRUE)
    endif()
endif()

if("${DEEPIN_OS_TYPE}" STREQUAL "")
    message(WARNING "No value of the \"--deepin-type\" in the process \"${DEEPIN_OS_RELEASE_TOOL}\"")
else()
    formatString(DEEPIN_OS_TYPE)

    message("Deepin OS Type: ${DEEPIN_OS_TYPE}")
    message("Deepin OS Version: ${DEEPIN_OS_VERSION}")

    if(NOT "${DEEPIN_OS_TYPE}" STREQUAL "")
        addDefinitions(Q_OS_DEEPIN_${DEEPIN_OS_TYPE})
        addDefinitions(DEEPIN_DDE)
        string(TOUPPER ${DEEPIN_OS_TYPE} DEEPIN_OS_TYPE)
        set(OS_DEEPIN_${DEEPIN_OS_TYPE} TRUE)
        set(DEEPIN_DDE TRUE)
    endif()

    formatString(DEEPIN_OS_VERSION)
    add_definitions(-DQ_OS_DEEPIN_VERSION=\"${DEEPIN_OS_VERSION}\")
endif()
