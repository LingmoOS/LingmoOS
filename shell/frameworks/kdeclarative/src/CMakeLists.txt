add_subdirectory(qmlcontrols)
add_subdirectory(calendarevents)

if(BUILD_QCH)
    ecm_add_qch(
        KF6Declarative_QCH
        NAME KDeclarative
        BASE_NAME KF6Declarative
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${CalendarEvents_QCH_SOURCES}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt6Core_QCH
        INCLUDE_DIRS
            ${CalendarEvents_QCH_INCLUDE_DIRS}
        BLANK_MACROS
            CALENDAREVENTS_EXPORT
            "CALENDAREVENTS_DEPRECATED_VERSION(x, y, t)"
            "CALENDAREVENTS_DEPRECATED_VERSION_BELATED(x, y, xt, yt, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()
