add_subdirectory(client)

add_subdirectory(tools)

ecm_qt_install_logging_categories(
    EXPORT KWAYLAND
    FILE kwayland.categories
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

    _make_absolute(KWaylandClient_APIDOX_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/client")

    ecm_add_qch(
        KWayland_QCH
        NAME KWayland
        BASE_NAME KWayland
        VERSION ${PROJECT_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KWaylandClient_APIDOX_SRCS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt6Gui_QCH
        INCLUDE_DIRS
            ${KWaylandClient_APIDOX_BUILD_INCLUDE_DIRS}
        BLANK_MACROS
            KWAYLANDCLIENT_EXPORT
            KWAYLANDCLIENT_DEPRECATED
            KWAYLANDCLIENT_DEPRECATED_EXPORT
            "KWAYLANDCLIENT_DEPRECATED_VERSION(x, y, t)"
            "KWAYLANDCLIENT_DEPRECATED_VERSION_BELATED(x, y, xt, yt, t)"
            "KWAYLANDCLIENT_ENUMERATOR_DEPRECATED_VERSION(x, y, t)"
            "KWAYLANDCLIENT_ENUMERATOR_DEPRECATED_VERSION_BELATED(x, y, xt, yt, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()
