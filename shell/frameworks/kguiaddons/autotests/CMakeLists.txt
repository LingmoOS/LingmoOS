find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMAddTests)

ecm_add_tests(
  kwordwraptest.cpp
  kcolorutilstest.cpp
  kiconutilstest.cpp
  kcursorsavertest.cpp
  kkeysequencerecordertest.cpp
  kurlhandlertest.cpp
  kcountryflagemojiiconenginetest.cpp
  LINK_LIBRARIES KF6::GuiAddons Qt6::Test
)
ecm_add_tests(kgeourihandlertest.cpp LINK_LIBRARIES Qt6::Test)
