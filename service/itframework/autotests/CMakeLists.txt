
find_package(Qt${QT_MAJOR_VERSION} ${REQUIRED_QT_VERSION} CONFIG REQUIRED Xml Test)

include(ECMAddTests)

ecm_add_tests(
   kactioncategorytest.cpp
   kactioncollectiontest.cpp
   LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test KF5::XmlGui
)
ecm_add_tests(
   kmainwindow_unittest.cpp
   ktoolbar_unittest.cpp
   ktooltiphelper_unittest.cpp
   kxmlgui_unittest.cpp
   GUI
   LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test KF5::IconThemes KF5::XmlGui
)

set_tests_properties(ktoolbar_unittest PROPERTIES RUN_SERIAL TRUE) # it wipes out ~/.qttest/share
