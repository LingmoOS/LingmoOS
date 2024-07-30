remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_TO_ASCII)

find_package(Qt6Test ${REQUIRED_QT_VERSION} CONFIG QUIET)

if(NOT Qt6Test_FOUND)
    message(STATUS "Qt6Test not found, autotests will not be built.")
    return()
endif()

include(ECMAddTests)

ecm_add_tests(
   kcompletioncoretest.cpp
   klineedit_unittest.cpp
   kcombobox_unittest.cpp
   ksortablelisttest.cpp
   kemailvalidatortest.cpp
   LINK_LIBRARIES Qt6::Test KF6::Completion
)
