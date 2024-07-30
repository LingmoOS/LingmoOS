
find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Xml Test)

include(ECMAddTests)

if (HAVE_QTDBUS)
    add_definitions(-DWITH_QTDBUS)
endif()

ecm_add_tests(
   kactioncategorytest.cpp
   kactioncollectiontest.cpp
   LINK_LIBRARIES Qt6::Test KF6::XmlGui
)
ecm_add_tests(
   kmainwindow_unittest.cpp
   ktoolbar_unittest.cpp
   ktooltiphelper_unittest.cpp
   kxmlgui_unittest.cpp
   GUI
   LINK_LIBRARIES Qt6::Test KF6::IconThemes KF6::XmlGui
)

set_tests_properties(ktoolbar_unittest PROPERTIES RUN_SERIAL TRUE) # it wipes out ~/.qttest/share
