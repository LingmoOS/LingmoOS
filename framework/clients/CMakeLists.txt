# Clients of system services, one library per directory, linking only
# the toolkit's base library. They form the LingmoClients package; a
# consumer asks for them by directory name:
#   find_package(LingmoClients REQUIRED COMPONENTS appearance)
#   target_link_libraries(app PRIVATE Lingmo::AppearanceClient)

find_package(LingmoSDK REQUIRED)

set(LINGMO_FRAMEWORK_CLIENTS "all" CACHE STRING
    "The clients/ directories to build: all, or a list of their names")

# lingmo_add_client(<name> TARGET <Target> SOURCES ... HEADERS ... [LIBRARIES ...])
# builds liblingmo-<name>-client as Lingmo::<Target>, its headers included
# as <lingmo/<name>/...>.
function(lingmo_add_client name)
    cmake_parse_arguments(arg "" "TARGET" "SOURCES;HEADERS;LIBRARIES" ${ARGN})
    lingmo_framework_add_library(lingmo-${name}-client
        TARGET ${arg_TARGET}
        MODULE ${name}
        EXPORT LingmoClient${name}Targets
        SOURCES ${arg_SOURCES}
        HEADERS ${arg_HEADERS}
        PUBLIC_LIBRARIES Lingmo::SDKBase ${arg_LIBRARIES}
    )
    lingmo_framework_export(LingmoClients LingmoClient${name}Targets)
endfunction()

file(GLOB client_directories LIST_DIRECTORIES true ${CMAKE_CURRENT_SOURCE_DIR}/*)
foreach(directory ${client_directories})
    get_filename_component(name ${directory} NAME)
    if(NOT EXISTS ${directory}/CMakeLists.txt)
        continue()
    endif()
    if(LINGMO_FRAMEWORK_CLIENTS STREQUAL "all" OR name IN_LIST LINGMO_FRAMEWORK_CLIENTS)
        add_subdirectory(${name})
    endif()
endforeach()

lingmo_framework_add_package(LingmoClients)
