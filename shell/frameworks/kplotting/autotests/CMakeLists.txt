
find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test Widgets)

include(ECMAddTests)

ecm_add_tests(
    kplotpointtest.cpp
    kplotaxistest.cpp
    kplotobjecttest.cpp
    kplotwidgettest.cpp
    LINK_LIBRARIES Qt6::Test KF6::Plotting
)
