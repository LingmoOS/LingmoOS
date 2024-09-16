# qt_helper.cmake

# 根据find_package拷贝所需要的qt库（目标文件未依赖的）

# 根据组件名获取库文件的目标
function(get_qt_library_target component_name target_var)
    if (TARGET Qt5::${component_name})
        set(${target_var} Qt5::${component_name} PARENT_SCOPE)
    elseif (TARGET Qt6::${component_name})
        set(${target_var} Qt6::${component_name} PARENT_SCOPE)
    else()
        message(FATAL_ERROR "Unknown Qt component: ${component_name}")
    endif()
endfunction()

# 项目使用
function(deployqt_qt_runtime arg_target)
    get_target_property(qt_qmake_executable Qt5::qmake IMPORTED_LOCATION)
    get_filename_component(qt_bin_directory "${qt_qmake_executable}" DIRECTORY)

    find_program(qt_deployqt_executable NAMES windeployqt macdeployqt HINTS "${qt_bin_directory}")

    if (NOT EXISTS "${qt_deployqt_executable}")
        message("can not find the deployqt tools")
        return()
    endif ()

    # 获取组件参数列表
    # ${ARGN}是一个内置变量，表示传递给函数或宏的所有未命名参数
    set(qt_components ${ARGN})

    if (WIN32)
        add_custom_command(TARGET ${arg_target} POST_BUILD COMMAND
            "${qt_deployqt_executable}"
            --libdir "$<TARGET_FILE_DIR:${arg_target}>"
            --plugindir "$<TARGET_FILE_DIR:${arg_target}>"
            --no-translations
#            --no-compiler-runtime
#            --no-system-d3d-compiler
            --no-opengl-sw
            --no-angle
            --no-webkit2
            $<$<CONFIG:Debug>:--pdb>
            "$<TARGET_FILE:${arg_target}>"
        )

        # 根据find_package后面的component拷贝需要的动态库(防止遗漏)
        foreach (component IN LISTS qt_components)
            get_qt_library_target(${component} qt_component_target)
            add_custom_command(TARGET ${arg_target} POST_BUILD COMMAND
                ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:${qt_component_target}>"
                "$<TARGET_FILE_DIR:${arg_target}>"
                )
        endforeach()
    endif ()
endfunction()
