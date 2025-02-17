
find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

include(ECMAddTests)

#TODO: add tests
#ecm_add_tests(
#   footest.cpp
#   LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test
#)
