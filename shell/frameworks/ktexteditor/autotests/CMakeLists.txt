find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)
add_definitions(-DQT_FORCE_ASSERTS=1)

include(ECMMarkAsTest)
include(ECMAddTests)

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

include_directories(
  # for config.h
  ${CMAKE_BINARY_DIR}

  # for generated ktexteditor headers
  ${CMAKE_BINARY_DIR}/src/include

  # for normal sources
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/include
  ${CMAKE_SOURCE_DIR}/src/buffer
  ${CMAKE_SOURCE_DIR}/src/completion
  ${CMAKE_SOURCE_DIR}/src/dialogs
  ${CMAKE_SOURCE_DIR}/src/document
  ${CMAKE_SOURCE_DIR}/src/script
  ${CMAKE_SOURCE_DIR}/src/mode
  ${CMAKE_SOURCE_DIR}/src/render
  ${CMAKE_SOURCE_DIR}/src/search
  ${CMAKE_SOURCE_DIR}/src/syntax
  ${CMAKE_SOURCE_DIR}/src/undo
  ${CMAKE_SOURCE_DIR}/src/utils
  ${CMAKE_SOURCE_DIR}/src/view
)

add_definitions(-DTEST_DATA_DIR=\"${CMAKE_SOURCE_DIR}/autotests/input/\")
add_definitions(-DJS_DATA_DIR=\"${CMAKE_SOURCE_DIR}/src/script/data/\")
add_definitions(-DJS_SCRIPTTESTER_DATA_DIR=\"${CMAKE_SOURCE_DIR}/src/scripttester/\")

set (KTEXTEDITOR_TEST_LINK_LIBS KF6TextEditor
  KF6::I18n
  KF6::GuiAddons
  KF6::SyntaxHighlighting
  KF6::Codecs
  KF6::Completion
  Qt6::Qml
)

if(ENABLE_PCH)
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/pch.cpp "/*empty file*/")
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/pch.h
        "#pragma once\n"
        "#include <QObject>\n"
        "#include <QTest>\n"
        "#include <QString>\n"
    )
    add_library(kte_test_pch STATIC ${CMAKE_CURRENT_BINARY_DIR}/pch.cpp)
    target_link_libraries(kte_test_pch ${KTEXTEDITOR_TEST_LINK_LIBS} Qt6::Test)
    target_precompile_headers(kte_test_pch PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/pch.h)
endif()

include(ECMMarkAsTest)

# test executable for encoding
add_executable(kateencodingtest src/kateencodingtest.cpp)
target_link_libraries(kateencodingtest ${KTEXTEDITOR_TEST_LINK_LIBS})
ecm_mark_as_test(kateencodingtest)

# test macro for encoding tests
MACRO(KTEXTEDITOR_ENCODING_TEST _encoding _testname)
  ADD_TEST (NAME encoding_${_testname}_create COMMAND kateencodingtest ${_encoding} ${CMAKE_SOURCE_DIR}/autotests/input/encoding/${_testname} ${CMAKE_CURRENT_BINARY_DIR}/${_testname} )
  ADD_TEST (NAME encoding_${_testname}_diff COMMAND ${CMAKE_COMMAND} -E compare_files ${CMAKE_SOURCE_DIR}/autotests/input/encoding/${_testname} ${CMAKE_CURRENT_BINARY_DIR}/${_testname} )
ENDMACRO(KTEXTEDITOR_ENCODING_TEST)

# add tests

# this file is utf-8, simple
KTEXTEDITOR_ENCODING_TEST ("utf-8" "utf8.txt")

# this file is latin15, but fallback should work!
KTEXTEDITOR_ENCODING_TEST ("utf-8" "latin15.txt")

# this file is utf32, little endian, but fallback should work!
KTEXTEDITOR_ENCODING_TEST ("utf-8" "utf32.txt")

# this file is utf16, little endian, but fallback should work!
KTEXTEDITOR_ENCODING_TEST ("utf-8" "utf16.txt")

# this file is utf32, big endian, but fallback should work!
KTEXTEDITOR_ENCODING_TEST ("utf-8" "utf32be.txt")

# this file is utf16, big endian, but fallback should work!
KTEXTEDITOR_ENCODING_TEST ("utf-8" "utf16be.txt")

# cyrillic utf-8
KTEXTEDITOR_ENCODING_TEST ("utf-8" "cyrillic_utf8.txt")

# cyrillic cp1251
KTEXTEDITOR_ENCODING_TEST ("utf-8" "cp1251.txt")

# cyrillic koi8-r
KTEXTEDITOR_ENCODING_TEST ("utf-8" "koi8-r.txt")

# one character latin-15 test, segfaulted
KTEXTEDITOR_ENCODING_TEST ("utf-8" "one-char-latin-15.txt")

# utf8 bom only, shouldn't produce error
#KTEXTEDITOR_ENCODING_TEST ("utf-8" "utf8-bom-only.txt") // TODO: make Kate::TextBuffer::saveBuffer save the BOM for empty files

# latin15, but coincidentally contains utf8 bom. The "bom" should be treated as normal text
KTEXTEDITOR_ENCODING_TEST ("ISO 8859-15" "latin15-with-utf8-bom.txt")

# test macro for indentation tests
MACRO(KTEXTEDITOR_INDENT_TEST _testfile _testname)
    ADD_TEST (
        NAME kateindenttest_${_testname}
        COMMAND ktexteditor_script_tester
            --indent-data-test ${CMAKE_CURRENT_SOURCE_DIR}/input/indent2/
            -fplaceholder
            ${CMAKE_CURRENT_SOURCE_DIR}/input/indent2/${_testfile}
    )
    set_tests_properties(kateindenttest_${_testname} PROPERTIES TIMEOUT 10)
ENDMACRO(KTEXTEDITOR_INDENT_TEST)

# test different indenters sepearately to have smaller test chunks, that takes LONG
KTEXTEDITOR_INDENT_TEST ("python.js" "testPython")
KTEXTEDITOR_INDENT_TEST ("julia.js" "testJulia")
KTEXTEDITOR_INDENT_TEST ("cstyle.js" "testCstyle")
KTEXTEDITOR_INDENT_TEST ("cppstyle.js" "testCppstyle")
KTEXTEDITOR_INDENT_TEST ("cmake.js" "testCMake")
KTEXTEDITOR_INDENT_TEST ("ruby.js" "testRuby")
KTEXTEDITOR_INDENT_TEST ("haskell.js" "testHaskell")
KTEXTEDITOR_INDENT_TEST ("latex.js" "testLatex")
KTEXTEDITOR_INDENT_TEST ("pascal.js" "testPascal")
KTEXTEDITOR_INDENT_TEST ("ada.js" "testAda")
KTEXTEDITOR_INDENT_TEST ("xml.js" "testXml")
KTEXTEDITOR_INDENT_TEST ("normal.js" "testNormal")
KTEXTEDITOR_INDENT_TEST ("replicode.js" "testReplicode")

add_test(
    NAME scripts_test
    COMMAND ktexteditor_script_tester
        ${CMAKE_CURRENT_SOURCE_DIR}/input/commands/
        ${CMAKE_CURRENT_SOURCE_DIR}/input/scripting/
)
set_tests_properties(scripts_test PROPERTIES TIMEOUT 10)

macro(ktexteditor_unit_test_offscreen testname)
    add_executable(${testname} src/${testname}.cpp ${ARGN})
    target_link_libraries(${testname} ${KTEXTEDITOR_TEST_LINK_LIBS} Qt6::Test)
    ecm_mark_as_test(${testname})
    if(ENABLE_PCH)
        target_precompile_headers(${testname} REUSE_FROM kte_test_pch)
    endif()
    ADD_TEST (NAME ${testname} COMMAND ${testname} ${OFFSCREEN_QPA})
endmacro()

ktexteditor_unit_test_offscreen(katetextbuffertest)
ktexteditor_unit_test_offscreen(range_test)
ktexteditor_unit_test_offscreen(cursorwords_test)
ktexteditor_unit_test_offscreen(undomanager_test)
ktexteditor_unit_test_offscreen(plaintextsearch_test)
ktexteditor_unit_test_offscreen(regexpsearch_test)
ktexteditor_unit_test_offscreen(scriptdocument_test)
ktexteditor_unit_test_offscreen(scripttester_test ../src/scripttester/scripttester.cpp)
ktexteditor_unit_test_offscreen(wordcompletiontest)
ktexteditor_unit_test_offscreen(searchbar_test)
ktexteditor_unit_test_offscreen(movingcursor_test)
ktexteditor_unit_test_offscreen(configinterface_test)
ktexteditor_unit_test_offscreen(kte_documentcursor)
ktexteditor_unit_test_offscreen(katedocument_test)
ktexteditor_unit_test_offscreen(movingrange_test)
ktexteditor_unit_test_offscreen(revision_test)
ktexteditor_unit_test_offscreen(modificationsystem_test)
ktexteditor_unit_test_offscreen(variable_test)
ktexteditor_unit_test_offscreen(templatehandler_test)
ktexteditor_unit_test_offscreen(camelcursortest)
ktexteditor_unit_test_offscreen(encodingtest)
ktexteditor_unit_test_offscreen(indentdetect_test)
ktexteditor_unit_test_offscreen(multicursortest)
ktexteditor_unit_test_offscreen(katemodemanager_test src/katemodemanager_test_base.cpp)
ktexteditor_unit_test_offscreen(kateview_test)
ktexteditor_unit_test_offscreen(inlinenote_test)
ktexteditor_unit_test_offscreen(completion_test src/codecompletiontestmodel.cpp src/codecompletiontestmodels.cpp)
ktexteditor_unit_test_offscreen(bug313759 src/testutils.cpp)
ktexteditor_unit_test_offscreen(bug205447 src/testutils.cpp)
ktexteditor_unit_test_offscreen(katefoldingtest)
ktexteditor_unit_test_offscreen(bug286887)
ktexteditor_unit_test_offscreen(bug313769)
ktexteditor_unit_test_offscreen(messagetest)

add_subdirectory(src/vimode)


#
# benchmarks, don't execute during normal testing
# they don't test additional stuff but only hang the CI
#
add_executable(katemodemanager_benchmark src/katemodemanager_test_base.cpp src/katemodemanager_benchmark.cpp)
ecm_mark_nongui_executable(katemodemanager_benchmark)
add_test(NAME katemodemanager_benchmark COMMAND katemodemanager_benchmark CONFIGURATIONS BENCHMARK)
target_link_libraries(katemodemanager_benchmark ${KTEXTEDITOR_TEST_LINK_LIBS} Qt6::Test)

add_executable(bench_search src/benchmarks/bench_search.cpp)
target_link_libraries(bench_search PRIVATE ${KTEXTEDITOR_TEST_LINK_LIBS})

add_executable(example src/example.cpp)
target_link_libraries(example PRIVATE ${KTEXTEDITOR_TEST_LINK_LIBS})
