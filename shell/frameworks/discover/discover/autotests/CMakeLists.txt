set(lingmo_discover_autotest_SRCS)
ecm_qt_declare_logging_category(lingmo_discover_autotest_SRCS HEADER discover_debug.h IDENTIFIER DISCOVER_LOG CATEGORY_NAME org.kde.lingmo.discover)
ecm_add_test(PaginateModelTest.cpp ../PaginateModel.cpp ${lingmo_discover_autotest_SRCS} TEST_NAME PaginateModelTest LINK_LIBRARIES Qt::Test)
target_include_directories(PaginateModelTest PUBLIC ${CMAKE_SOURCE_DIR}/libdiscover/)

if(BUILD_DummyBackend)
    add_test(NAME toplevels COMMAND Lingmo::Discover --test "${CMAKE_CURRENT_SOURCE_DIR}/toplevels.qml")
    add_test(NAME install COMMAND Lingmo::Discover --test "${CMAKE_CURRENT_SOURCE_DIR}/install.qml")

    add_test(NAME appstreamUrl COMMAND Lingmo::Discover --test "${CMAKE_CURRENT_SOURCE_DIR}/appstreamUrl.qml" "dummy://techie1")
    add_test(NAME missingResource COMMAND Lingmo::Discover --test "${CMAKE_CURRENT_SOURCE_DIR}/missingResource.qml" "dummy://caca")
    add_test(NAME apparg COMMAND Lingmo::Discover --test "${CMAKE_CURRENT_SOURCE_DIR}/appstreamUrl.qml" --application "dummy://techie1")
    add_test(NAME categoryarg COMMAND Lingmo::Discover --test "${CMAKE_CURRENT_SOURCE_DIR}/categoryArg.qml" --category "dummy 2.1")
    add_test(NAME wrongInput COMMAND Lingmo::Discover --test "${CMAKE_CURRENT_SOURCE_DIR}/wrongInput.qml" "CMakeLists.txt")
    add_test(NAME packageArgument COMMAND Lingmo::Discover --test "${CMAKE_CURRENT_SOURCE_DIR}/packageArgument.qml"
                                                           --local-filename "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt")
    add_test(NAME updateandinstall COMMAND Lingmo::Discover --test "${CMAKE_CURRENT_SOURCE_DIR}/updateandinstall.qml")

#   Just make sure they exit successfully
    add_test(NAME listbackends COMMAND Lingmo::Discover --listbackends)
    add_test(NAME listmodes COMMAND Lingmo::Discover --listmodes)
endif()
