# TODO
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMMarkAsTest)

add_executable(kbookmarkdialogtest kbookmarkdialogtest.cpp)
ecm_mark_as_test(kbookmarkdialogtest)
target_link_libraries(kbookmarkdialogtest KF6::Bookmarks KF6::BookmarksWidgets)
