
include(ECMAddTests)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED Test)

include_directories(../src)

ecm_add_test(iconmodeltest.cpp ../src/iconmodel.cpp ../src/sortfiltermodel.cpp
   TEST_NAME iconmodeltest
   LINK_LIBRARIES
      Qt::Gui
      Qt::Test
      KF6::KIOWidgets
      KF6::ConfigCore
      KF6::IconThemes
      Plasma::Plasma
      KF6::ConfigWidgets
)
