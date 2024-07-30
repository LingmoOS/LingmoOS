# make sure the generated headers can be found
include_directories(${CMAKE_CURRENT_BINARY_DIR})

include(ECMMarkAsTest)
include(${CMAKE_SOURCE_DIR}/KF6ConfigMacros.cmake)

macro(GEN_KCFG_TEST_SOURCE _testName _srcs)
    KCONFIG_ADD_KCFG_FILES(${_srcs} ${_testName}.kcfgc ${ARGN})
endmacro()

function(kconfig_compiler_test ARG_NAME)
    cmake_parse_arguments(PARSE_ARGV 1 ARG "GENERATE_MOC" "MAIN;CLASS_NAME;KCFG;KCFGC" "")

    string(TOLOWER "${ARG_NAME}" _name_lc)

    if("${ARG_MAIN}" STREQUAL "")
        set(ARG_MAIN "${_name_lc}main.cpp")
    endif()

    if("${ARG_CLASS_NAME}" STREQUAL "")
        set(ARG_CLASS_NAME "${ARG_NAME}")
    endif()

    if("${ARG_KCFG}" STREQUAL "")
        set(ARG_KCFG "${_name_lc}.kcfg")
    endif()

    if("${ARG_KCFGC}" STREQUAL "")
        set(ARG_KCFGC "${_name_lc}.kcfgc")
    endif()

    if(${ARG_GENERATE_MOC})
        set(_moc GENERATE_MOC)
    endif()

    # execute the tests in ${CMAKE_CURRENT_BINARY_DIR} to ensure we can easily find the generated files

    ecm_add_test(TEST_NAME ${ARG_NAME} ${ARG_MAIN} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    KCONFIG_ADD_KCFG_FILES(${ARG_NAME} "${ARG_KCFGC}" ${_moc})
    target_link_libraries(${ARG_NAME} Qt6::Test Qt6::Qml KF6::ConfigGui)

    ecm_add_test(TEST_NAME ${ARG_NAME}_cmake ${ARG_MAIN} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    get_filename_component(_kcfg_we "${ARG_KCFG}" NAME_WE)
    configure_file("${ARG_KCFG}" "${_kcfg_we}_cmake.kcfg" COPYONLY)
    kconfig_target_kcfg_file(${ARG_NAME}_cmake FILE "${CMAKE_CURRENT_BINARY_DIR}/${_kcfg_we}_cmake.kcfg" CLASS_NAME "${ARG_CLASS_NAME}" ${_moc} ${ARG_UNPARSED_ARGUMENTS})
    target_link_libraries(${ARG_NAME}_cmake Qt6::Test Qt6::Qml KF6::ConfigGui)
    add_dependencies(${ARG_NAME}_cmake ${ARG_NAME})
endfunction()

kconfig_compiler_test(Test1 PARENT_IN_CONSTRUCTOR MUTATORS)
kconfig_compiler_test(Test2 MUTATORS GLOBAL_ENUMS ITEM_ACCESSORS SET_USER_TEXTS INHERITS MyPrefs INCLUDE_FILES myprefs.h MEMBER_VARIABLES public)
kconfig_compiler_test(Test3 MUTATORS GLOBAL_ENUMS ITEM_ACCESSORS NAME_SPACE TestNameSpace)
kconfig_compiler_test(Test3a MUTATORS GLOBAL_ENUMS ITEM_ACCESSORS NAME_SPACE TestNameSpace::InnerNameSpace)
kconfig_compiler_test(Test4 SINGLETON MUTATORS)
kconfig_compiler_test(Test5 PARENT_IN_CONSTRUCTOR SINGLETON MUTATORS GLOBAL_ENUMS)
kconfig_compiler_test(Test6 MUTATORS GLOBAL_ENUMS)
kconfig_compiler_test(Test7 MUTATORS GLOBAL_ENUMS)

kconfig_compiler_test(Test8 KCFGC test8a.kcfgc KCFG test8a.kcfg CLASS_NAME Test8a PARENT_IN_CONSTRUCTOR MUTATORS)
KCONFIG_ADD_KCFG_FILES(Test8 test8c.kcfgc)
KCONFIG_ADD_KCFG_FILES(Test8 test8b.kcfgc)
kconfig_target_kcfg_file(Test8_cmake FILE test8b.kcfg CLASS_NAME Test8b SINGLETON MUTATORS INCLUDE_FILES test8a.h INHERITS Test8a)
kconfig_target_kcfg_file(Test8_cmake FILE test8c.kcfg CLASS_NAME Test8c SINGLETON PARENT_IN_CONSTRUCTOR MUTATORS)

kconfig_compiler_test(Test9 MEMBER_VARIABLES public MUTATORS)
kconfig_compiler_test(Test10 SINGLETON)

kconfig_compiler_test(Test11 PARENT_IN_CONSTRUCTOR MUTATORS INHERITS MyPrefs INCLUDE_FILES myprefs.h MEMBER_VARIABLES public ITEM_ACCESSORS SET_USER_TEXTS USE_ENUM_TYPES DEFAULT_VALUE_GETTERS)
KCONFIG_ADD_KCFG_FILES(Test11 test11a.kcfgc)
kconfig_target_kcfg_file(Test11_cmake FILE test11a.kcfg CLASS_NAME Test11a MUTATORS INHERITS MyPrefs INCLUDE_FILES myprefs.h MEMBER_VARIABLES public ITEM_ACCESSORS SET_USER_TEXTS USE_ENUM_TYPES DEFAULT_VALUE_GETTERS)

kconfig_compiler_test(Test12 test12main.cpp)
kconfig_compiler_test(Test13 test13main.cpp MUTATORS GENERATE_PROPERTIES GENERATE_MOC)
kconfig_compiler_test(Test14 KCFGC test14.kcfgc KCFG test14.kcfg CLASS_NAME Test14 GENERATE_PROPERTIES GENERATE_MOC SINGLETON QML_REGISTRATION)
kconfig_compiler_test(Test15 CLASS_NAME Test15 GENERATE_PROPERTIES GENERATE_MOC SINGLETON QML_REGISTRATION)

kconfig_compiler_test(test_state_config MAIN test_state_config_main.cpp CLASS_NAME MyStateConfig MUTATORS ITEM_ACCESSORS GENERATE_MOC)
kconfig_compiler_test(test_emptyentries MAIN test_emptyentries_main.cpp CLASS_NAME QMakeBuilderSettings SINGLETON GENERATE_MOC)
kconfig_compiler_test(test_subgroups MAIN test_subgroups_main.cpp CLASS_NAME TestSubgroups NOTIFIERS GENERATE_MOC)
kconfig_compiler_test(test_dpointer MAIN test_dpointer_main.cpp CLASS_NAME TestDPointer SINGLETON MUTATORS MEMBER_VARIABLES dpointer ITEM_ACCESSORS SET_USER_TEXTS)
kconfig_compiler_test(test_signal MAIN test_signal_main.cpp CLASS_NAME TestSignal SINGLETON MUTATORS MEMBER_VARIABLES private GENERATE_MOC)
kconfig_compiler_test(test_notifiers MAIN test_notifiers_main.cpp CLASS_NAME TestNotifiers MUTATORS GLOBAL_ENUMS NOTIFIERS)

########### next target ###############

set(kconfigcompiler_test_signals_SRCS kconfigcompiler_test_signals.cpp)
gen_kcfg_test_source(signals_test_singleton kconfigcompiler_test_signals_SRCS GENERATE_MOC)
gen_kcfg_test_source(signals_test_no_singleton kconfigcompiler_test_signals_SRCS GENERATE_MOC)
gen_kcfg_test_source(signals_test_singleton_dpointer kconfigcompiler_test_signals_SRCS GENERATE_MOC)
gen_kcfg_test_source(signals_test_no_singleton_dpointer kconfigcompiler_test_signals_SRCS GENERATE_MOC)
gen_kcfg_test_source(signals_test_singleton_itemaccessors kconfigcompiler_test_signals_SRCS GENERATE_MOC)

ecm_add_test(${kconfigcompiler_test_signals_SRCS}
    TEST_NAME kconfigcompiler-signals-test
    LINK_LIBRARIES Qt6::Test KF6::ConfigGui
)

########### next target ###############

set(test_properties_minmax_SRCS test_properties_minmax_main.cpp)
gen_kcfg_test_source(test_properties_minmax test_properties_minmax_SRCS GENERATE_MOC)

ecm_add_test(${test_properties_minmax_SRCS}
    TEST_NAME test_properties_minmax
    LINK_LIBRARIES KF6::ConfigGui
)

########### next target ###############

set(test_param_minmax_SRCS test_param_minmax_main.cpp)
gen_kcfg_test_source(test_param_minmax test_param_minmax_SRCS)

ecm_add_test(${test_param_minmax_SRCS}
    TEST_NAME test_param_minmax
    LINK_LIBRARIES KF6::ConfigGui
)

########### next target ###############

ecm_add_test(kconfigcompiler_test.cpp
    TEST_NAME kconfigcompiler-basic-test
    LINK_LIBRARIES Qt6::Test
)


########### next target ###############

set(test_qcategory_SRCS test_qdebugcategorymain.cpp test_qdebugcategory_debug.cpp)
gen_kcfg_test_source(test_qdebugcategory test_qcategory_SRCS)
ecm_add_test(TEST_NAME test_qdebugcategory ${test_qcategory_SRCS})
target_link_libraries(test_qdebugcategory KF6::ConfigGui)

########### next target ###############

set(test_translation_qt_SRCS test_translation_qt_main.cpp)
gen_kcfg_test_source(test_translation_qt test_translation_qt_SRCS)

ecm_add_test(TEST_NAME test_translation_qt ${test_translation_qt_SRCS})
target_link_libraries(test_translation_qt KF6::ConfigGui)

########### next target ###############

set(test_translation_kde_SRCS test_translation_kde_main.cpp)
gen_kcfg_test_source(test_translation_kde test_translation_kde_SRCS)

ecm_add_test(TEST_NAME test_translation_kde ${test_translation_kde_SRCS})
target_link_libraries(test_translation_kde KF6::ConfigGui)

########### next target ###############

set(test_translation_kde_domain_SRCS test_translation_kde_domain_main.cpp)
gen_kcfg_test_source(test_translation_kde_domain test_translation_kde_domain_SRCS)

ecm_add_test(TEST_NAME test_translation_kde_domain ${test_translation_kde_domain_SRCS})
target_link_libraries(test_translation_kde_domain KF6::ConfigGui)

########### next target ###############

set(test_fileextensions_SRCS test_fileextensions_main.cxx)
gen_kcfg_test_source(test_fileextensions test_fileextensions_SRCS)

ecm_add_test(TEST_NAME test_fileextensions ${test_fileextensions_SRCS})
target_link_libraries(test_fileextensions KF6::ConfigGui)


####### next target ##########
## If this to compiles, we are sure that the generation is correct.
set(test_enums_and_properties_SRCS enum_group_main.cpp)
gen_kcfg_test_source(test_enums_and_properties test_enums_and_properties_SRCS GENERATE_MOC)
ecm_add_test(TEST_NAME test_enums_and_properties ${test_enums_and_properties_SRCS})
target_link_libraries(test_enums_and_properties KF6::ConfigGui)

ecm_add_test(test14qml.cpp TEST_NAME test14qml)
qt6_add_qml_module(test14qml URI "org.kde.kconfig.test14" QML_FILES Test14Qml.qml)
kconfig_add_kcfg_files(test14qml "test14.kcfgc" GENERATE_MOC)
target_link_libraries(test14qml PRIVATE Qt6::Gui Qt6::Qml Qt6::Test KF6::ConfigGui)

ecm_add_test(test15qml.cpp TEST_NAME test15qml)
qt6_add_qml_module(test15qml URI "org.kde.kconfig.test15" QML_FILES Test15Qml.qml)
kconfig_add_kcfg_files(test15qml "test15.kcfgc" GENERATE_MOC)
target_link_libraries(test15qml PRIVATE Qt6::Gui Qt6::Qml Qt6::Test KF6::ConfigGui)
