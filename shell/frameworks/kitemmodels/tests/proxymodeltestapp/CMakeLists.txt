add_executable(proxymodeltestapp)

target_sources(proxymodeltestapp PRIVATE
    main.cpp
    mainwindow.cpp

    breadcrumbdirectionwidget.cpp
    breadcrumbnavigationwidget.cpp
    breadcrumbswidget.cpp
    checkablewidget.cpp
    descendantpmwidget.cpp
    kidentityproxymodelwidget.cpp
    kreparentingproxymodel.cpp
    lessthanwidget.cpp
    matchcheckingwidget.cpp
    modelcommanderwidget.cpp
    proxyitemselectionwidget.cpp
    proxymodeltestwidget.cpp
    selectionpmwidget.cpp
    # statesaverwidget.cpp
)

if (TARGET Qt6::Qml)
    target_sources(proxymodeltestapp PRIVATE
        reparentingpmwidget.cpp
        scriptablereparentingwidget.cpp
    )
    target_link_libraries(proxymodeltestapp Qt6::Qml)
endif()

if (TARGET Qt6::QuickWidgets)
    target_sources(proxymodeltestapp PRIVATE
        selectioninqmlwidget.cpp
        descendantqmltree.cpp
    )
    target_link_libraries(proxymodeltestapp Qt6::QuickWidgets)
    target_compile_definitions(proxymodeltestapp PRIVATE -DSRC_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
endif()

target_link_libraries(proxymodeltestapp
    KF6::ItemModels
    proxymodeltestsuite
    Qt6::Widgets
)
