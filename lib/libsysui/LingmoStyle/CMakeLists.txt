set(MODULE_NAME LingmoStyle)
set(MODULE_VERSION 3.0)

unset(qml_files)
unset(resource_files)
unset(QML_PATHS)
unset(RES_PATHS)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

qt_add_library(${MODULE_NAME} SHARED)

set(SINGLETON_FILES "")

#遍历所有qml文件
file(GLOB_RECURSE QML_PATHS *.qml qmldir)
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
qt_add_qml_module(${MODULE_NAME}
    PLUGIN_TARGET ${MODULE_NAME}
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/temp/QtQuick/Controls/LingmoStyle
    VERSION ${MODULE_VERSION}
    URI "QtQuick.Controls.LingmoStyle"
    TYPEINFO "plugins.qmltypes"
    QML_FILES ${qml_files}
    RESOURCES ${resource_files}
    RESOURCE_PREFIX "/style"
)

install(DIRECTORY ${CMAKE_BINARY_DIR}/temp/QtQuick/Controls/LingmoStyle DESTINATION ${INSTALL_QMLDIR}/QtQuick/Controls)
install(TARGETS ${MODULE_NAME} DESTINATION ${INSTALL_QMLDIR}/QtQuick/Controls/${MODULE_NAME})
