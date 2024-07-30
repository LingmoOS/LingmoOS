remove_definitions(-DQT_NO_CAST_FROM_ASCII)

find_package(Qt6Test ${REQUIRED_QT_VERSION} CONFIG QUIET)

if(NOT TARGET Qt6::Test)
    message(STATUS "Qt6Test not found, autotests will not be built.")
    return()
endif()

include(ECMAddTests)

ecm_add_test(kbookmarktest.cpp TEST_NAME kbookmarktest LINK_LIBRARIES KF6::Bookmarks KF6::BookmarksWidgets Qt6::Test)
