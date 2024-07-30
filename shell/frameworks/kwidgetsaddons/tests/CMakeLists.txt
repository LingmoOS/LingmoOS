
include(ECMMarkAsTest)

find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

macro(kwidgetsaddons_executable_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} Qt6::Test KF6::WidgetsAddons)
    ecm_mark_as_test(${_testname})
  endforeach(_testname)
endmacro()

kwidgetsaddons_executable_tests(
  kanimatedbuttontest
  kbusyindicatorwidgettest
  kcharselecttest
  kcollapsiblegroupboxtest
  kdatecomboboxtestapp
  kdatepicktest
  kdatepickerpopuptest
  kdatetimeedittestapp
  kgradientselectortest
  kledtest
  kmessageboxtest
  kmessageboxwidtest
  kselectactiontest
  kseparatortest
  ksqueezedtextlabeltest
  ktitlewidgettest
  kfontchooserdialogtest
  kfontrequestertest
  kpassworddialogtest
  keditlistwidgettest
  kratingwidgettest
  kactionselectortest
  kcolorcombotest
  kpixmapregionselectordialogtest
  ktoolbarlabelactiontest
  kpagedialogtest
  kpagewidgettest
  kassistantdialogtest
  kmessagewidgettest
  kmessagedialogtest
  knewpassworddialogtest
  knewpasswordwidget_test
  kpixmapsequenceoverlaypaintertest
  kmimetypechoosertest
  ksplittercollapserbuttongui_test
  ktooltipwidget_test
  kpasswordlineedit_test
  ktwofingertap_test
  ktwofingerswipe_test
)

add_executable(kdatetabletest kdatetabletest.cpp ../src/kdatetable.cpp)
target_include_directories(kdatetabletest PRIVATE ../src)
target_link_libraries(kdatetabletest Qt6::Widgets)
ecm_mark_as_test(kdatetabletest)

add_executable(kcolumnresizertestapp)
set(kcolumnresizertestapp_UI_SRCS)
qt_wrap_ui(kcolumnresizertestapp_UI_SRCS kcolumnresizertestapp.ui)
target_sources(kcolumnresizertestapp PRIVATE ${kcolumnresizertestapp_UI_SRCS} kcolumnresizertestapp.cpp)
target_link_libraries(kcolumnresizertestapp KF6::WidgetsAddons Qt6::Widgets)
ecm_mark_as_test(kcolumnresizertestapp)
