include(ECMAddTests)

find_package(Qt6 ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED Test)
set_package_properties(Qt6Test PROPERTIES PURPOSE "Required for autotests")

ecm_add_tests(
  configtest.cpp
  persontest.cpp
  providertest.cpp
  LINK_LIBRARIES Qt6::Test KF6::Attica
)
