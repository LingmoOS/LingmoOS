configure_file(test-config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/test-config.h)
if(TARGET KF6UserFeedbackConsole)
    add_library(KF6UserFeedbackTestUtils STATIC
        servercontroller.cpp servercontroller.h
    )
    target_link_libraries(KF6UserFeedbackTestUtils LINK_PUBLIC Qt6::Test KF6UserFeedbackConsole)
endif()

function(uf_add_test _file)
    get_filename_component(_name ${_file} NAME_WE)
    add_executable(${_name} ${_file})
    target_link_libraries(${_name} Qt6::Test ${ARGN})
    add_test(NAME ${_name} COMMAND ${_name})
endfunction()

uf_add_test(datasourcetest.cpp KF6UserFeedbackCore Qt6::Gui) # needs Gui for ScreenInfoSource to work
uf_add_test(openglinfosourcetest.cpp KF6UserFeedbackCore Qt6::Gui)
uf_add_test(providertest.cpp KF6UserFeedbackCore Qt6::Gui)
uf_add_test(selectionratiosourcetest.cpp KF6UserFeedbackCore Qt6::Gui)
uf_add_test(surveyprovidertest.cpp KF6UserFeedbackCore)

uf_add_test(feedbackconfigtest.cpp KF6UserFeedbackWidgets)

if(HAVE_SURVEY_TARGET_EXPRESSIONS)
    uf_add_test(surveytargetexpressiontest.cpp KF6UserFeedbackCommon)
endif()

if(TARGET KF6UserFeedbackConsole)
    uf_add_test(producttest.cpp KF6UserFeedbackConsole)
    uf_add_test(schematemplatetest.cpp KF6UserFeedbackConsole)
    uf_add_test(sampletest.cpp KF6UserFeedbackConsole)
    uf_add_test(serverinfotest.cpp KF6UserFeedbackConsole)

    uf_add_test(schemamodeltest.cpp KF6UserFeedbackTestUtils)
    uf_add_test(datamodeltest.cpp KF6UserFeedbackTestUtils)
    uf_add_test(timeaggregationmodeltest.cpp KF6UserFeedbackTestUtils)
    uf_add_test(categoryaggregationmodeltest.cpp KF6UserFeedbackTestUtils)
    uf_add_test(numericaggregationmodeltest.cpp KF6UserFeedbackTestUtils)
    uf_add_test(ratiosetaggregationmodeltest.cpp KF6UserFeedbackTestUtils)
endif()

if(ENABLE_PHP)
    list(FIND PHP_MODULES "PDO" _has_pdo)
    if(TARGET KF6UserFeedbackConsole AND _has_pdo GREATER -1)
        uf_add_test(productapitest.cpp KF6UserFeedbackTestUtils)
        uf_add_test(productmodeltest.cpp KF6UserFeedbackTestUtils)
        uf_add_test(surveyapitest.cpp KF6UserFeedbackTestUtils)
        uf_add_test(submittest.cpp KF6UserFeedbackTestUtils KF6UserFeedbackCore Qt6::Gui)
    endif()

    set(php_test_srcs
        utilstest.php
        schemaentrytest.php
        schemaentryelementtest.php
        producttest.php
        sampletest.php
        surveytest.php
        aggregationtest.php
    )

    php_lint(datastoretesthelper.php)
    php_lint(${php_test_srcs})
    if(PhpUnit_FOUND)
        foreach(_php_test ${php_test_srcs})
            add_test(NAME ${_php_test} COMMAND ${PHPUNIT_EXECUTABLE} ${_php_test} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
        endforeach()
    endif()
endif()
