qt_standard_project_setup()

# Create Qt6 library target
qt_add_library(${TARGET}.GraphicalEffects SHARED)

target_include_directories(${TARGET}.GraphicalEffects PRIVATE ${Qt6Quick_PRIVATE_INCLUDE_DIRS})


target_link_libraries(${TARGET}.GraphicalEffects
        PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        )


# Clear the list of source files
set(sources_files "")
set(qml_files "")
set(resource_files "")

#遍历所有Cpp文件
file(GLOB_RECURSE CPP_FILES *.cpp *.h)
foreach (filepath ${CPP_FILES})
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" filename ${filepath})
    list(APPEND sources_files ${filename})
endforeach (filepath)

set(SINGLETON_FILES "Controls/LingmoRouter.qml" "Controls/LingmoUnits.qml")

#遍历所有qml文件
file(GLOB_RECURSE QML_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/*.qml qmldir)
foreach (filepath ${QML_PATHS})
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" filename ${filepath})
    if (${filename} MATCHES "qmldir")
        list(APPEND resource_files ${filename})
    else ()
        list(APPEND qml_files ${filename})
        # Check for singleton
        foreach (singletion_name ${SINGLETON_FILES})
            if (${filename} MATCHES ${singletion_name})
                set_source_files_properties(${filename} PROPERTIES QT_QML_SINGLETON_TYPE true)
            endif ()
        endforeach (singletion_name)
    endif ()
endforeach (filepath)

#遍历所有资源文件
set(RES_SUFFIX "")
set(RES_PATHS "")
list(APPEND RES_SUFFIX *.png *.jpg *.svg *.ico *.ttf *.webp *.js *.qm *.qsb)
list(APPEND RES_PATHS ${CMAKE_CURRENT_SOURCE_DIR})
foreach (respath ${RES_PATHS})
    foreach (filesuffix ${RES_SUFFIX})
        file(GLOB_RECURSE RES_FOUND ${respath}/${filesuffix})
        foreach (filepath ${RES_FOUND})
            string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" filename ${filepath})
            list(APPEND resource_files ${filename})
        endforeach()
    endforeach ()
endforeach (respath)

# Set Qt6 QMl Plugins
qt_add_qml_module(${TARGET}.GraphicalEffects
    PLUGIN_TARGET ${TARGET}.GraphicalEffects
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/temp/${PROJECT_NAME}/GraphicalEffects
    VERSION ${MODULE_VERSION_MAJOR}.${MODULE_VERSION_MINOR}
    URI ${PROJECT_NAME}.GraphicalEffects
    TYPEINFO "plugins.qmltypes"
    SOURCES ${sources_files}
    QML_FILES ${qml_files}
    RESOURCES ${resource_files}
    RESOURCE_PREFIX "/lingmoui/kit/GraphicalEffects"
)

set_target_properties(${TARGET}.GraphicalEffects
    PROPERTIES  VERSION ${MODULE_VERSION} SOVERSION ${MODULE_VERSION_MAJOR})


install(TARGETS ${TARGET}.GraphicalEffects EXPORT ${TARGET}Targets DESTINATION ${INSTALL_QMLDIR}/${PROJECT_NAME}/GraphicalEffects)
