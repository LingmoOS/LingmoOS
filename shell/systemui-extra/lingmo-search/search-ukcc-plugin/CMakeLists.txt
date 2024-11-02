set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core DBus Widgets LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core DBus Widgets LinguistTools REQUIRED)
find_package(PkgConfig REQUIRED)
set(UKCC_PLUGIN_EXTERNAL_LIBS "")
set(UKCC_PLUGIN_PC_PKGS gsettings-qt lingmosdk-qtwidgets lingmo-ai-base)

foreach(PC_LIB IN ITEMS ${UKCC_PLUGIN_PC_PKGS})
    pkg_check_modules(${PC_LIB} REQUIRED ${PC_LIB})
    if(${${PC_LIB}_FOUND})
        include_directories(${${PC_LIB}_INCLUDE_DIRS})
        link_directories(${${PC_LIB}_LIBRARY_DIRS})
        list(APPEND UKCC_PLUGIN_EXTERNAL_LIBS PkgConfig::${PC_LIB})
    endif()
endforeach()

file(GLOB UKCC_PLUGIN_TS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/translations/*.ts)
set_source_files_properties(${UKCC_PLUGIN_TS_FILES} PROPERTIES OUTPUT_LOCATION ${CMAKE_BINARY_DIR}/search-ukcc-plugin/.qm)
qt5_create_translation(UKCC_PLUGIN_QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${UKCC_PLUGIN_TS_FILES})

add_library(search-ukcc-plugin MODULE
        search.cpp search.h
        ${UKCC_PLUGIN_QM_FILES}
        )
target_link_libraries(search-ukcc-plugin PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        gsettings-qt
        ukcc
        ${UKCC_PLUGIN_PC_PKGS}
        )

install(TARGETS search-ukcc-plugin
        DESTINATION /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/lingmo-control-center)
install(FILES
        ${UKCC_PLUGIN_TS_FILES}
        ${UKCC_PLUGIN_QM_FILES}
        DESTINATION /usr/share/lingmo-search/search-ukcc-plugin/translations)
file(GLOB IMG_FILES ./image/*)
install(FILES ${IMG_FILES} DESTINATION /usr/share/lingmo-search/search-ukcc-plugin/image)
