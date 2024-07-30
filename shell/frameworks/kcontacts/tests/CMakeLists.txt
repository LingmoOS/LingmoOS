
include(ECMMarkAsTest)

########### next target ###############

# FIXME: Build fails
add_executable(testread testread.cpp)
target_link_libraries(testread KF6Contacts KF6::I18n Qt6::Gui)

########### next target ###############

# FIXME: Build fails
add_executable(testwrite testwrite.cpp)
target_link_libraries(testwrite KF6Contacts Qt6::Gui KF6::I18n)

########### next target ###############

add_executable(testread2 testread2.cpp testutils.cpp)
target_link_libraries(testread2 KF6Contacts Qt6::Gui)
