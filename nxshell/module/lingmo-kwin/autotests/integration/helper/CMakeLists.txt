add_executable(copy copy.cpp)
target_link_libraries(copy Qt${QT_MAJOR_VERSION}::Gui)
ecm_mark_as_test(copy)
######################
add_executable(paste paste.cpp)
target_link_libraries(paste Qt${QT_MAJOR_VERSION}::Gui)
ecm_mark_as_test(paste)
######################
add_executable(kill kill.cpp)
target_link_libraries(kill Qt${QT_MAJOR_VERSION}::Widgets)
ecm_mark_as_test(kill)
