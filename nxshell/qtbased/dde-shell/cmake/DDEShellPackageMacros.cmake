include(CMakeParseArguments)

macro(ds_build_package)
    set(oneValueArgs PACKAGE TARGET PACKAGE_ROOT_DIR)
    cmake_parse_arguments(_config "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(package_root_dir ${CMAKE_CURRENT_SOURCE_DIR}/package)
    if (DEFINED _config_PACKAGE_ROOT_DIR)
        set(package_root_dir ${_config_PACKAGE_ROOT_DIR})
    endif()
    file(GLOB_RECURSE package_files ${package_root_dir}/*)
    add_custom_target(${_config_PACKAGE}_package ALL
        SOURCES ${package_files}
    )
    set(package_dirs ${PROJECT_BINARY_DIR}/packages/${_config_PACKAGE}/)
    add_custom_command(TARGET ${_config_PACKAGE}_package
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${package_root_dir} ${package_dirs}
    )

    if (DEFINED _config_TARGET)
        set_target_properties(${_config_TARGET} PROPERTIES
            PREFIX ""
            OUTPUT_NAME ${_config_PACKAGE}
            LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/plugins/
        )
        # avoid warning when `D_APPLET_CLASS` used in .cpp
        set_property(TARGET ${_config_TARGET} APPEND PROPERTY AUTOMOC_MACRO_NAMES "D_APPLET_CLASS")
    endif()
endmacro()

function(ds_install_package)
    ds_build_package(${ARGN})
    install(DIRECTORY ${package_dirs} DESTINATION ${DDE_SHELL_PACKAGE_INSTALL_DIR}/${_config_PACKAGE})

    if (DEFINED _config_TARGET)
        install(TARGETS ${_config_TARGET} DESTINATION ${DDE_SHELL_PLUGIN_INSTALL_DIR}/)
    endif()
endfunction()

function(ds_handle_package_translation)
    set(oneValueArgs PACKAGE)
    set(multiValueArgs QML_FILES SOURCE_FILES )
    cmake_parse_arguments(_config "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT _config_QML_FILES)
        file(GLOB_RECURSE _config_QML_FILES ${CMAKE_CURRENT_SOURCE_DIR}/package/*.qml)
        file(GLOB additional_QML_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.qml)
        list(APPEND _config_QML_FILES ${additional_QML_FILES})
    endif()

    if(NOT _config_SOURCE_FILES)
        file(GLOB _config_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
    endif()

    set(TRANSLATION_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/translations/${_config_PACKAGE}.ts
        ${CMAKE_CURRENT_SOURCE_DIR}/translations/${_config_PACKAGE}_az.ts
        ${CMAKE_CURRENT_SOURCE_DIR}/translations/${_config_PACKAGE}_bo.ts
        ${CMAKE_CURRENT_SOURCE_DIR}/translations/${_config_PACKAGE}_ca.ts
        ${CMAKE_CURRENT_SOURCE_DIR}/translations/${_config_PACKAGE}_es.ts
        ${CMAKE_CURRENT_SOURCE_DIR}/translations/${_config_PACKAGE}_fi.ts
        ${CMAKE_CURRENT_SOURCE_DIR}/translations/${_config_PACKAGE}_fr.ts
        ${CMAKE_CURRENT_SOURCE_DIR}/translations/${_config_PACKAGE}_hu.ts
        ${CMAKE_CURRENT_SOURCE_DIR}/translations/${_config_PACKAGE}_it.ts
        ${CMAKE_CURRENT_SOURCE_DIR}/translations/${_config_PACKAGE}_ja.ts
        ${CMAKE_CURRENT_SOURCE_DIR}/translations/${_config_PACKAGE}_ko.ts
        ${CMAKE_CURRENT_SOURCE_DIR}/translations/${_config_PACKAGE}_nb_NO.ts
        ${CMAKE_CURRENT_SOURCE_DIR}/translations/${_config_PACKAGE}_pl.ts
        ${CMAKE_CURRENT_SOURCE_DIR}/translations/${_config_PACKAGE}_pt_BR.ts
        ${CMAKE_CURRENT_SOURCE_DIR}/translations/${_config_PACKAGE}_ru.ts
        ${CMAKE_CURRENT_SOURCE_DIR}/translations/${_config_PACKAGE}_uk.ts
        ${CMAKE_CURRENT_SOURCE_DIR}/translations/${_config_PACKAGE}_zh_CN.ts
        ${CMAKE_CURRENT_SOURCE_DIR}/translations/${_config_PACKAGE}_zh_HK.ts
        ${CMAKE_CURRENT_SOURCE_DIR}/translations/${_config_PACKAGE}_zh_TW.ts
    )

    set(package_dirs ${PROJECT_BINARY_DIR}/packages/${_config_PACKAGE}/)

    # FIXME: not working on Qt 6.7
    # set_source_files_properties(${TRANSLATION_FILES}
    #     PROPERTIES OUTPUT_LOCATION "${package_dirs}/translations"
    # )

    add_custom_target(${_config_PACKAGE}_translation ALL
        SOURCES ${TRANSLATION_FILES}
    )

    qt_add_translations(${_config_PACKAGE}_translation
        TS_FILES ${TRANSLATION_FILES}
        SOURCES ${_config_QML_FILES} ${_config_SOURCE_FILES}
        QM_FILES_OUTPUT_VARIABLE TRANSLATED_FILES
    )

    # /usr/share/dde-shell/org.deepin.xxx/translations/org.deepin.xxx.qm
    install(FILES ${TRANSLATED_FILES} DESTINATION ${DDE_SHELL_TRANSLATION_INSTALL_DIR}/${_config_PACKAGE}/translations)
endfunction()
