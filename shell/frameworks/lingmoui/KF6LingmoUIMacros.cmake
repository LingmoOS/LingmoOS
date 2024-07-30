include(CMakeParseArguments)
include(ExternalProject)


function(lingmoui_package_ocean_icons)
    set(_multiValueArgs ICONS)
    cmake_parse_arguments(ARG "" "" "${_multiValueArgs}" ${ARGN} )

    if(NOT ARG_ICONS)
        message(FATAL_ERROR "No ICONS argument given to lingmoui_package_ocean_icons")
    endif()

    #include icons used by LingmoUI components themselves
    set(ARG_ICONS ${ARG_ICONS}
        application-exit
        dialog-close
        dialog-error
        dialog-information
        dialog-positive
        dialog-warning
        edit-clear-locationbar-ltr
        edit-clear-locationbar-rtl
        edit-copy
        edit-delete-remove
        emblem-error
        emblem-information
        emblem-positive
        emblem-warning
        globe
        go-next
        go-next-symbolic
        go-next-symbolic-rtl
        go-previous
        go-previous-symbolic
        go-previous-symbolic-rtl
        go-up
        handle-sort
        mail-sent
        open-menu-symbolic
        overflow-menu-left
        overflow-menu-right
        overflow-menu
        password-show-off
        password-show-on
        tools-report-bug
        user
        view-left-new
        view-right-new
        view-left-close
        view-right-close
    )

    function(_find_ocean_icon icon varName)
        #HACKY
        SET(path "")
        file(GLOB_RECURSE path ${_OCEANICONS_DIR}/icons/*/48/${icon}.svg )

        #search in other sizes as well
        if (path STREQUAL "")
            file(GLOB_RECURSE path ${_OCEANICONS_DIR}/icons/*/32/${icon}.svg )
            if (path STREQUAL "")
                file(GLOB_RECURSE path ${_OCEANICONS_DIR}/icons/*/22/${icon}.svg )
                if (path STREQUAL "")
                    file(GLOB_RECURSE path ${_OCEANICONS_DIR}/icons/*/16/${icon}.svg )
                endif()
            endif()
        endif()
        if (path STREQUAL "")
            file(GLOB_RECURSE path ${_OCEANICONS_DIR}/icons/*/symbolic/${icon}.svg )
        endif()
        if (path STREQUAL "")
            return()
        endif()

        list(LENGTH path _count_paths)
        if (_count_paths GREATER 1)
            message(WARNING "Found more than one version of '${icon}': ${path}")
        endif()
        list(GET path 0 path)
        get_filename_component(path "${path}" REALPATH)

        SET(${varName} ${path} PARENT_SCOPE)
    endfunction()

    if (OCEANICONS_DIR AND NOT EXISTS ${OCEANICONS_DIR})
        message(FATAL_ERROR "OCEANICONS_DIR variable does not point to existing dir: \"${OCEANICONS_DIR}\"")
    endif()

    set(_OCEANICONS_DIR "${OCEANICONS_DIR}")

    #FIXME: this is a terrible hack
    if(NOT _OCEANICONS_DIR)
        set(_OCEANICONS_DIR "${CMAKE_BINARY_DIR}/ocean-icons/src/ocean-icons")

        # replacement for ExternalProject_Add not yet working
        # first time config?
        if (NOT EXISTS ${_OCEANICONS_DIR})
            find_package(Git)
            execute_process(COMMAND ${GIT_EXECUTABLE} clone --depth 1 https://invent.kde.org/frameworks/ocean-icons.git ${_OCEANICONS_DIR})
        endif()

        # external projects are only pulled at make time, not configure time
        # so this is too late to work with the _find_ocean_icon() method
        # _find_ocean_icon() would need to be turned into a target/command
        if (FALSE)
        ExternalProject_Add(
            ocean-icons
            PREFIX ocean-icons
            GIT_REPOSITORY https://invent.kde.org/frameworks/ocean-icons.git
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ""
            INSTALL_COMMAND ""
            LOG_DOWNLOAD ON
        )
        endif()
    endif()

    message (STATUS "Found external ocean icons:")
    foreach(_iconName ${ARG_ICONS})
        set(_iconPath "")
        _find_ocean_icon(${_iconName} _iconPath)
        message (STATUS ${_iconPath})
        if (EXISTS ${_iconPath})
            install(FILES ${_iconPath} DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/lingmoui/ocean-internal/icons/ RENAME ${_iconName}.svg)
        endif()
    endforeach()

    #generate an index.theme that qiconloader can understand
    file(WRITE ${CMAKE_BINARY_DIR}/index.theme "[Icon Theme]\nName=Ocean\nDirectories=icons\n[icons]\nSize=32\nType=Scalable")
    install(FILES ${CMAKE_BINARY_DIR}/index.theme DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/lingmoui/ocean-internal/)
endfunction()
