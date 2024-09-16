add_subdirectory(tools)
add_subdirectory(client)
add_subdirectory(server)

ecm_qt_install_logging_categories(
    EXPORT DWAYLAND
    FILE dwayland.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

if(BUILD_QCH)
    macro(_make_absolute var_name base_path)
        set(_result)
        foreach(_path ${${var_name}})
            if(IS_ABSOLUTE "${_path}")
                list(APPEND _result "${_path}")
            else()
                list(APPEND _result "${base_path}/${_path}")
            endif()
        endforeach()
        set(${var_name} ${_result})
    endmacro()

    _make_absolute(DWaylandClient_APIDOX_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/client")
    _make_absolute(DWaylandServer_APIDOX_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/server")

    ecm_add_qch(
        DWayland_QCH
        NAME DWayland
        BASE_NAME DWayland
        VERSION ${PROJECT_VERSION}
        ORG_DOMAIN com.deepin
        SOURCES # using only public headers, to cover only public API
            ${DWaylandClient_APIDOX_SRCS}
            ${DWaylandServer_APIDOX_SRCS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt5Gui_QCH
        INCLUDE_DIRS
            ${DWaylandClient_APIDOX_BUILD_INCLUDE_DIRS}
            ${DWaylandServer_APIDOX_BUILD_INCLUDE_DIRS}
        BLANK_MACROS
            DWAYLANDCLIENT_EXPORT
            DWAYLANDCLIENT_DEPRECATED
            DWAYLANDCLIENT_DEPRECATED_EXPORT
            "DWAYLANDCLIENT_DEPRECATED_VERSION(x, y, t)"
            "DWAYLANDCLIENT_DEPRECATED_VERSION_BELATED(x, y, xt, yt, t)"
            "DWAYLANDCLIENT_ENUMERATOR_DEPRECATED_VERSION(x, y, t)"
            "DWAYLANDCLIENT_ENUMERATOR_DEPRECATED_VERSION_BELATED(x, y, xt, yt, t)"
            DWAYLANDSERVER_EXPORT
            DWAYLANDSERVER_DEPRECATED
            DWAYLANDSERVER_DEPRECATED_EXPORT
            "DWAYLANDSERVER_DEPRECATED_VERSION(x, y, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()
