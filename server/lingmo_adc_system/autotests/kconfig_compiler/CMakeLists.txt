
# On Windows we have to generate the .h and .cpp inside ${CMAKE_BINARY_DIR}/bin because
# otherwise QFINDTESTDATA will not be able to locate them.
if(WIN32)
    set(KCFG_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin")
else()
    set(KCFG_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
endif()

# make sure the generated headers can be found
include_directories(${KCFG_OUTPUT_DIR})

include(${CMAKE_SOURCE_DIR}/KF5ConfigMacros.cmake)
macro(GEN_KCFG_TEST_SOURCE _testName _srcs)
   KCONFIG_ADD_KCFG_FILES(${_srcs} ${_testName}.kcfgc ${ARGN})
endmacro()

include(ECMMarkAsTest)

########### next target ###############

set(test1_SRCS test1main.cpp )


gen_kcfg_test_source(test1 test1_SRCS)

ecm_add_test(TEST_NAME test1 ${test1_SRCS})
target_link_libraries(test1 KF5::ConfigGui)


########### next target ###############

set(test2_SRCS test2main.cpp )


gen_kcfg_test_source(test2 test2_SRCS)

ecm_add_test(TEST_NAME test2 ${test2_SRCS})
target_link_libraries(test2 KF5::ConfigGui)


########### next target ###############

set(test3_SRCS test3main.cpp )


gen_kcfg_test_source(test3 test3_SRCS)

ecm_add_test(TEST_NAME test3 ${test3_SRCS})
target_link_libraries(test3 KF5::ConfigGui)


########### next target ###############

set(test3a_SRCS test3amain.cpp )


gen_kcfg_test_source(test3a test3a_SRCS)

ecm_add_test(TEST_NAME test3a ${test3a_SRCS})
target_link_libraries(test3a KF5::ConfigGui)


########### next target ###############

set(test4_SRCS test4main.cpp )


gen_kcfg_test_source(test4 test4_SRCS)

ecm_add_test(TEST_NAME test4 ${test4_SRCS})
target_link_libraries(test4 KF5::ConfigGui)


########### next target ###############

set(test5_SRCS test5main.cpp )


gen_kcfg_test_source(test5 test5_SRCS)

ecm_add_test(TEST_NAME test5 ${test5_SRCS})
target_link_libraries(test5 KF5::ConfigGui)


########### next target ###############

set(test6_SRCS test6main.cpp )


gen_kcfg_test_source(test6 test6_SRCS)

ecm_add_test(TEST_NAME test6 ${test6_SRCS})
target_link_libraries(test6 KF5::ConfigGui)


########### next target ###############

set(test7_SRCS test7main.cpp )


gen_kcfg_test_source(test7 test7_SRCS)

ecm_add_test(TEST_NAME test7 ${test7_SRCS})
target_link_libraries(test7 KF5::ConfigGui)


########### next target ###############

set(test8_SRCS test8main.cpp )


gen_kcfg_test_source(test8a test8_SRCS)
gen_kcfg_test_source(test8b test8_SRCS)
gen_kcfg_test_source(test8c test8_SRCS)

ecm_add_test(TEST_NAME test8 ${test8_SRCS})
target_link_libraries(test8 KF5::ConfigGui)


########### next target ###############

set(test9_SRCS test9main.cpp )


gen_kcfg_test_source(test9 test9_SRCS)

ecm_add_test(TEST_NAME test9 ${test9_SRCS})
target_link_libraries(test9 KF5::ConfigGui)


########### next target ###############

set(test10_SRCS test10main.cpp )


gen_kcfg_test_source(test10 test10_SRCS)

ecm_add_test(TEST_NAME test10 ${test10_SRCS})
target_link_libraries(test10 KF5::ConfigGui)


########### next target ###############

set(test11_SRCS test11main.cpp )


gen_kcfg_test_source(test11 test11_SRCS)
gen_kcfg_test_source(test11a test11_SRCS)

ecm_add_test(TEST_NAME test11 ${test11_SRCS})
target_link_libraries(test11 KF5::ConfigGui)


########### next target ###############

set(test12_SRCS test12main.cpp )

gen_kcfg_test_source(test12 test12_SRCS)

ecm_add_test(TEST_NAME test12 ${test12_SRCS})
target_link_libraries(test12 KF5::ConfigGui)

########### next target ###############

set(test13_SRCS test13main.cpp )

gen_kcfg_test_source(test13 test13_SRCS GENERATE_MOC)

ecm_add_test(TEST_NAME test13 ${test13_SRCS})
target_link_libraries(test13 KF5::ConfigGui)

########### next target ###############

set(test_state_config_SRCS test_state_config_main.cpp)

gen_kcfg_test_source(test_state_config test_state_config_SRCS GENERATE_MOC)

ecm_add_test(TEST_NAME test_state_config ${test_state_config_SRCS})
target_link_libraries(test_state_config KF5::ConfigGui Qt${QT_MAJOR_VERSION}::Test)

########### next target ###############

set(test_emptyentries_SRCS test_emptyentries_main.cpp )

gen_kcfg_test_source(test_emptyentries test_emptyentries_SRCS GENERATE_MOC)

ecm_add_test(TEST_NAME test_emptyentries ${test_emptyentries_SRCS})
target_link_libraries(test_emptyentries KF5::ConfigGui)

########### next target ###############

set(test_subgroups_SRCS test_subgroups_main.cpp )

gen_kcfg_test_source(test_subgroups test_subgroups_SRCS GENERATE_MOC)

ecm_add_test(TEST_NAME test_subgroups ${test_subgroups_SRCS})
target_link_libraries(test_subgroups KF5::ConfigGui)


########### next target ###############

set(test_dpointer_SRCS test_dpointer_main.cpp )

gen_kcfg_test_source(test_dpointer test_dpointer_SRCS)

ecm_add_test(TEST_NAME test_dpointer ${test_dpointer_SRCS})
target_link_libraries(test_dpointer KF5::ConfigGui)


########### next target ###############

set(test_signal_SRCS test_signal_main.cpp )
gen_kcfg_test_source(test_signal test_signal_SRCS GENERATE_MOC)
ecm_add_test(TEST_NAME test_signal ${test_signal_SRCS})
target_link_libraries(test_signal KF5::ConfigGui)


########### next target ###############

set(test_notifiers_SRCS test_notifiers_main.cpp )
gen_kcfg_test_source(test_notifiers test_notifiers_SRCS)
ecm_add_test(TEST_NAME test_notifiers ${test_notifiers_SRCS})
target_link_libraries(test_notifiers KF5::ConfigGui)


########### next target ###############

set(kconfigcompiler_test_signals_SRCS kconfigcompiler_test_signals.cpp)
gen_kcfg_test_source(signals_test_singleton kconfigcompiler_test_signals_SRCS GENERATE_MOC)
gen_kcfg_test_source(signals_test_no_singleton kconfigcompiler_test_signals_SRCS GENERATE_MOC)
gen_kcfg_test_source(signals_test_singleton_dpointer kconfigcompiler_test_signals_SRCS GENERATE_MOC)
gen_kcfg_test_source(signals_test_no_singleton_dpointer kconfigcompiler_test_signals_SRCS GENERATE_MOC)
gen_kcfg_test_source(signals_test_singleton_itemaccessors kconfigcompiler_test_signals_SRCS GENERATE_MOC)

ecm_add_test(${kconfigcompiler_test_signals_SRCS}
    TEST_NAME kconfigcompiler-signals-test
    LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test KF5::ConfigGui
)

########### next target ###############

set(test_properties_minmax_SRCS test_properties_minmax_main.cpp)
gen_kcfg_test_source(test_properties_minmax test_properties_minmax_SRCS GENERATE_MOC)

ecm_add_test(${test_properties_minmax_SRCS}
    TEST_NAME test_properties_minmax
    LINK_LIBRARIES KF5::ConfigGui
)

########### next target ###############

set(test_param_minmax_SRCS test_param_minmax_main.cpp)
gen_kcfg_test_source(test_param_minmax test_param_minmax_SRCS)

ecm_add_test(${test_param_minmax_SRCS}
    TEST_NAME test_param_minmax
    LINK_LIBRARIES KF5::ConfigGui
)

########### next target ###############

ecm_add_test(kconfigcompiler_test.cpp
    TEST_NAME kconfigcompiler-basic-test
    LINK_LIBRARIES Qt${QT_MAJOR_VERSION}::Test
)


########### next target ###############

set(test_qcategory_SRCS test_qdebugcategorymain.cpp test_qdebugcategory_debug.cpp)
gen_kcfg_test_source(test_qdebugcategory test_qcategory_SRCS)
ecm_add_test(TEST_NAME test_qdebugcategory ${test_qcategory_SRCS})
target_link_libraries(test_qdebugcategory KF5::ConfigGui)

########### next target ###############

set(test_translation_qt_SRCS test_translation_qt_main.cpp)
gen_kcfg_test_source(test_translation_qt test_translation_qt_SRCS)

ecm_add_test(TEST_NAME test_translation_qt ${test_translation_qt_SRCS})
target_link_libraries(test_translation_qt KF5::ConfigGui)

########### next target ###############

set(test_translation_kde_SRCS test_translation_kde_main.cpp)
gen_kcfg_test_source(test_translation_kde test_translation_kde_SRCS)

ecm_add_test(TEST_NAME test_translation_kde ${test_translation_kde_SRCS})
target_link_libraries(test_translation_kde KF5::ConfigGui)

########### next target ###############

set(test_translation_kde_domain_SRCS test_translation_kde_domain_main.cpp)
gen_kcfg_test_source(test_translation_kde_domain test_translation_kde_domain_SRCS)

ecm_add_test(TEST_NAME test_translation_kde_domain ${test_translation_kde_domain_SRCS})
target_link_libraries(test_translation_kde_domain KF5::ConfigGui)

########### next target ###############

set(test_fileextensions_SRCS test_fileextensions_main.cxx)
gen_kcfg_test_source(test_fileextensions test_fileextensions_SRCS)

ecm_add_test(TEST_NAME test_fileextensions ${test_fileextensions_SRCS})
target_link_libraries(test_fileextensions KF5::ConfigGui)


####### next target ##########
## If this to compiles, we are sure that the generation is correct.
set(test_enums_and_properties_SRCS enum_group_main.cpp)
gen_kcfg_test_source(test_enums_and_properties test_enums_and_properties_SRCS GENERATE_MOC)
ecm_add_test(TEST_NAME test_enums_and_properties ${test_enums_and_properties_SRCS})
target_link_libraries(test_enums_and_properties KF5::ConfigGui)
