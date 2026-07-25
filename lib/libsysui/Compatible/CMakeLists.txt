qt_standard_project_setup()

# Create Qt6 library target
qt_add_library(${TARGET}CompatibleModule SHARED)

set(TARGET_SOURCES
        iconthemeprovider.cpp
        thememanager.cpp

        newiconitem.cpp
        managedtexturenode.cpp
        wheelhandler.cpp
        qqmlsortfilterproxymodel.cpp

        desktop/menupopupwindow.cpp

        lingmoui.cpp
        )

target_include_directories(${TARGET}CompatibleModule PRIVATE desktop)

if (CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
    list(APPEND TARGET_SOURCES
            platforms/windows/windowhelper.cpp platforms/windows/windowhelper.h
            platforms/windows/blurhelper/windowblur.cpp platforms/windows/blurhelper/windowblur.h
            platforms/windows/shadowhelper/boxshadowrenderer.cpp platforms/windows/shadowhelper/boxshadowrenderer.h
            platforms/windows/shadowhelper/tileset.cpp platforms/windows/shadowhelper/tileset.h
            platforms/windows/shadowhelper/windowshadow.cpp platforms/windows/shadowhelper/windowshadow.h
            )
    target_include_directories(${TARGET}CompatibleModule PRIVATE platforms/windows)
    target_include_directories(${TARGET}CompatibleModule PRIVATE platforms/windows/shadowhelper)
    target_include_directories(${TARGET}CompatibleModule PRIVATE platforms/windows/blurhelper)
elseif (APPLE)
    list(APPEND TARGET_SOURCES
            platforms/mac/windowhelper.cpp platforms/mac/windowhelper.h
            platforms/mac/blurhelper/windowblur.cpp platforms/mac/blurhelper/windowblur.h
            platforms/mac/shadowhelper/boxshadowrenderer.cpp platforms/mac/shadowhelper/boxshadowrenderer.h
            platforms/mac/shadowhelper/tileset.cpp platforms/mac/shadowhelper/tileset.h
            platforms/mac/shadowhelper/windowshadow.cpp platforms/mac/shadowhelper/windowshadow.h
            )
    target_include_directories(${TARGET}CompatibleModule PRIVATE platforms/mac)
    target_include_directories(${TARGET}CompatibleModule PRIVATE platforms/mac/shadowhelper)
    target_include_directories(${TARGET}CompatibleModule PRIVATE platforms/mac/blurhelper)
else ()
    if (KF6WindowSystem_FOUND)
        list(APPEND TARGET_SOURCES
        platforms/linux/windowhelper.cpp platforms/linux/windowhelper.h
        platforms/linux/shadowhelper/boxshadowrenderer.cpp platforms/linux/shadowhelper/boxshadowrenderer.h
        platforms/linux/shadowhelper/tileset.cpp platforms/linux/shadowhelper/tileset.h
        platforms/linux/shadowhelper/windowshadow.cpp platforms/linux/shadowhelper/windowshadow.h
        )
    endif()
    list(APPEND TARGET_SOURCES
            platforms/linux/blurhelper/windowblur.cpp platforms/linux/blurhelper/windowblur.h
            )
    target_include_directories(${TARGET}CompatibleModule PRIVATE platforms/linux)
    target_include_directories(${TARGET}CompatibleModule PRIVATE platforms/linux/shadowhelper)
    target_include_directories(${TARGET}CompatibleModule PRIVATE platforms/linux/blurhelper)
endif ()

target_link_libraries(${TARGET}CompatibleModule
        PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::DBus
        Qt6::Widgets
        PRIVATE
        Qt6::Qml
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::GuiPrivate
        Qt6::Core5Compat
        )

if (UNIX)
    if (NOT APPLE)
        target_link_libraries(${TARGET}CompatibleModule PRIVATE Qt6::Gui Qt6::GuiPrivate)
        if (KF6WindowSystem_FOUND)
            target_link_libraries(${TARGET}CompatibleModule PRIVATE KF6::WindowSystem)
            add_compile_definitions(KF_WINDOWSYSTEM_FOUND)
        endif ()
    endif ()
endif ()

# Clear the list of source files
set(sources_files "")
set(qml_files "")
set(resource_files "")

set(SINGLETON_FILES Controls/Theme.qml Controls/Units.qml)

#遍历所有qml文件
file(GLOB_RECURSE QML_PATHS Controls/*.qml Controls/*.qml qmldir)
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
file(GLOB_RECURSE RES_PATHS *.png *.jpg *.svg *.ico *.ttf *.webp *.js *.qm)
foreach (filepath ${RES_PATHS})
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" filename ${filepath})
    list(APPEND resource_files ${filename})
endforeach (filepath)

# Set Qt6 QMl Plugins
qt_add_qml_module(${TARGET}CompatibleModule
    PLUGIN_TARGET ${TARGET}CompatibleModule
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/temp/${PROJECT_NAME}/CompatibleModule
    VERSION ${MODULE_VERSION_MAJOR}.${MODULE_VERSION_MINOR}
    URI ${PROJECT_NAME}.CompatibleModule
    TYPEINFO "plugins.qmltypes"
    SOURCES ${TARGET_SOURCES} 
    QML_FILES ${qml_files}
    RESOURCES ${resource_files}
    RESOURCE_PREFIX "/lingmoui/kit/compatible_module"
    NO_GENERATE_PLUGIN_SOURCE
)

set_target_properties(${TARGET}CompatibleModule
    PROPERTIES  VERSION ${MODULE_VERSION} SOVERSION ${MODULE_VERSION_MAJOR})

install(DIRECTORY ${CMAKE_BINARY_DIR}/temp/${PROJECT_NAME}/CompatibleModule DESTINATION ${INSTALL_QMLDIR}/${PROJECT_NAME})
generate_export_header(${TARGET}CompatibleModule BASE_NAME ${TARGET}CompatibleModule)

install(TARGETS ${TARGET}CompatibleModule EXPORT ${TARGET}Targets DESTINATION ${INSTALL_QMLDIR}/${PROJECT_NAME}/CompatibleModule)

add_subdirectory(lingmo-style)
