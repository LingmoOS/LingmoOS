
include(ECMAddTests)

find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

ecm_add_tests(
  kacceleratormanagertest.cpp
  kcharselect_unittest.cpp
  kcollapsiblegroupbox_test.cpp
  kcolorbuttontest.cpp
  kdatecomboboxtest.cpp
  kdatepickerautotest.cpp
  kdatepickerpopupautotest.cpp
  kdatetimeedittest.cpp
  kdualactiontest.cpp
  kpixmapsequencewidgettest.cpp
  knewpasswordwidgettest.cpp
  kselectaction_unittest.cpp
  ksqueezedtextlabelautotest.cpp
  ktimecomboboxtest.cpp
  ktooltipwidgettest.cpp
  kmessagewidgetautotest.cpp
  kpagedialogautotest.cpp
  kpassworddialogautotest.cpp
  kpasswordlineedittest.cpp
  ksplittercollapserbuttontest.cpp
  kmultitabbartest.cpp
  ktwofingertaptest.cpp
  ktwofingerswipetest.cpp
  klineediteventhandlertest.cpp
  LINK_LIBRARIES Qt6::Test KF6::WidgetsAddons
)

set (CMAKE_AUTOUIC TRUE)
ecm_add_test(
  kcolumnresizertest.cpp
  kcolumnresizertest-forms.ui
  kcolumnresizertest-grids.ui
  kcolumnresizertest-grid-and-form.ui
  TEST_NAME kcolumnresizertest
  NAME_PREFIX "kwidgetsaddons-"
  LINK_LIBRARIES Qt6::Test KF6::WidgetsAddons
)
