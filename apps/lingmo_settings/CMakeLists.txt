cmake_minimum_required(VERSION 3.16)

project(lingmo-settings
    VERSION 1.0
    LANGUAGES CXX
)

# The pages use lower-case prefixes for the framework clients;
# namespace_compat.h aliases them at global scope.
add_compile_options(-include${CMAKE_CURRENT_SOURCE_DIR}/src/namespace_compat.h)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)
include(CTest)

find_package(LingmoSDK REQUIRED)

# Where the pictures are looked up; a scratch build that is never installed
# can point it at src/ instead.
set(SETTINGS_DATA_DIR ${CMAKE_INSTALL_FULL_DATADIR}/lingmo-settings CACHE PATH "Settings' installed data")

# Each page lives in src/pages/<page>/ and registers itself, so pages are
# added without touching this file. A build may take some of them only.
set(SETTINGS_PAGES "all" CACHE STRING "The src/pages directories to build: all, or a list of their names")
file(GLOB page_directories LIST_DIRECTORIES true ${PROJECT_SOURCE_DIR}/src/pages/*)
set(SETTINGS_BUILT_PAGES)
set(SETTINGS_PAGE_SOURCES)
foreach(directory ${page_directories})
    get_filename_component(page ${directory} NAME)
    if(IS_DIRECTORY ${directory} AND (SETTINGS_PAGES STREQUAL "all" OR page IN_LIST SETTINGS_PAGES))
        list(APPEND SETTINGS_BUILT_PAGES ${page})
        file(GLOB_RECURSE sources CONFIGURE_DEPENDS ${directory}/*.cpp)
        list(APPEND SETTINGS_PAGE_SOURCES ${sources})
    endif()
endforeach()

# Objects rather than an archive: a page is only referenced by its static
# registration, which a linker would drop from an archive.
add_library(settings-ui OBJECT
    src/widgets/card.cpp
    src/widgets/colors.cpp
    src/widgets/formdialog.cpp
    src/widgets/iconbutton.cpp
    src/widgets/imageselectionitem.cpp
    src/widgets/listrow.cpp
    src/widgets/metrics.cpp
    src/widgets/picture.cpp
    src/widgets/rowcombobox.cpp
    src/widgets/rowicon.cpp
    src/widgets/searchfield.cpp
    src/widgets/sections.cpp
    src/widgets/sliderrow.cpp
    src/widgets/standardbutton.cpp
    src/widgets/tilegrid.cpp
    src/window/options.cpp
    src/window/page.cpp
    src/window/pagelist.cpp
    src/window/settingswindow.cpp
    src/window/sidebar.cpp
    ${SETTINGS_PAGE_SOURCES}
)
target_include_directories(settings-ui PUBLIC src)
target_compile_definitions(settings-ui PUBLIC SETTINGS_DATA_DIR="${SETTINGS_DATA_DIR}")
target_link_libraries(settings-ui PUBLIC Lingmo::SDK)

# A page's own dependencies: src/pages/<page>/page.cmake finds and links
# them to settings-ui.
foreach(page ${SETTINGS_BUILT_PAGES})
    if(EXISTS ${PROJECT_SOURCE_DIR}/src/pages/${page}/page.cmake)
        include(${PROJECT_SOURCE_DIR}/src/pages/${page}/page.cmake)
    endif()
endforeach()

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE settings-ui)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY src/images/ DESTINATION ${SETTINGS_DATA_DIR}/images)

lingmo_add_translations(lingmo-settings SOURCES src DESKTOP_FILES lingmo-settings.desktop)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/lingmo-settings.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)

install(FILES
    src/datetime/60-lingmo-datetime.rules
    DESTINATION ${CMAKE_INSTALL_DATADIR}/polkit-1/rules.d
)

if(BUILD_TESTING)
    add_subdirectory(tests/ui)
endif()
