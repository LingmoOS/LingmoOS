if(NOT TARGET Qt6::QuickTest)
    message(STATUS "Qt6QuickTest not found, autotests will not be built.")
    return()
endif()

add_executable(qmltest qmltest.cpp actiondata.cpp)
qt_add_qml_module(qmltest URI LingmoUITestUtils)
target_link_libraries(qmltest PRIVATE Qt6::Qml Qt6::QuickTest)

if (BUILD_SHARED_LIBS)
    target_link_libraries(qmltest PRIVATE LingmoUI)
else()
    target_link_libraries(qmltest PRIVATE LingmoUIplugin)
endif()

macro(lingmoui_add_tests)
    if (WIN32)
        set(_extra_args -platform offscreen)
    endif()

    if (BUILD_SHARED_LIBS)
        set(_extra_args ${_extra_args} -import ${CMAKE_BINARY_DIR}/bin)
    endif()

    foreach(test ${ARGV})
        add_test(NAME ${test}
                 COMMAND qmltest
                        ${_extra_args}
                        -input ${test}
                 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
    endforeach()
endmacro()

lingmoui_add_tests(
    tst_action.qml
    tst_actiontoolbar.qml
    tst_card.qml
    tst_colorutils.qml
    tst_columnview.qml
    tst_delegates.qml
    tst_dialogs.qml
    tst_formlayout.qml
    tst_globaldrawer.qml
    tst_headerfooterlayout.qml
    tst_icon.qml
    tst_ImageColors.qml
    tst_inlinemessage.qml
    tst_inlineviewheader.qml
    tst_keynavigation.qml
    tst_listskeynavigation.qml
    tst_menudialog.qml
    tst_mnemonicdata.qml
    tst_navigationtabbar.qml
    tst_overlaysheet.qml
    tst_overlayzstacking.qml
    tst_padding.qml
    tst_pagerow.qml
    tst_placeholdermessage.qml
    tst_sceneposition.qml
    tst_scrollablepage.qml
    tst_spellcheck.qml
    tst_theme.qml

    mobile/tst_pagerow.qml

    pagepool/tst_layers.qml
    pagepool/tst_pagepool.qml

    wheelhandler/tst_filterMouseEvents.qml
    wheelhandler/tst_invokables.qml
    wheelhandler/tst_onWheel.qml
    wheelhandler/tst_scrolling.qml
)

set_tests_properties(
    tst_actiontoolbar.qml
    tst_theme.qml

    PROPERTIES
        ENVIRONMENT "QT_QUICK_CONTROLS_STYLE=Basic;LINGMOUI_FORCE_STYLE=1"
)

set_tests_properties(
    mobile/tst_pagerow.qml

    PROPERTIES
        ENVIRONMENT "QT_QUICK_CONTROLS_MOBILE=1"
)
