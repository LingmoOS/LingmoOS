ecm_add_test(
    fakebacktracegenerator.cpp
    backtraceparsertest.cpp
    TEST_NAME backtraceparsertest
    LINK_LIBRARIES Qt::Test Qt::Core KF6::KIOWidgets drkonqi_backtrace_parser
)
add_executable(backtraceparsertest_manual fakebacktracegenerator.cpp backtraceparsertest_manual.cpp)
target_link_libraries(backtraceparsertest_manual Qt::Core KF6::I18n KF6::KIOWidgets drkonqi_backtrace_parser)
ecm_mark_as_test(backtraceparsertest_manual)
ecm_mark_nongui_executable(backtraceparsertest_manual)
