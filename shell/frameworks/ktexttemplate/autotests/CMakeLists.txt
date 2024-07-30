find_package(Qt6Test ${REQUIRED_QT_VERSION} REQUIRED)

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

set( KTEXTTEMPLATE_PLUGIN_PATH "${CMAKE_BINARY_DIR}/" )

configure_file(ktexttemplate_paths.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/ktexttemplate_paths.h)

find_package(Qt6LinguistTools QUIET)
if (Qt6LinguistTools_FOUND)
  set(TESTS_LOCALES "fr_FR" "de_DE")

  set(QT_LUPDATE_EXECUTABLE Qt6::lupdate)
  set(QT_LRELEASE_EXECUTABLE Qt6::lrelease)

  set(LINGUIST_TS_ARGS COMMAND "${QT_LUPDATE_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/test_input.cpp" "-ts")

  foreach(TESTS_LOCALE ${TESTS_LOCALES})
      set( LINGUIST_TS_ARGS ${LINGUIST_TS_ARGS} "${CMAKE_CURRENT_SOURCE_DIR}/test_${TESTS_LOCALE}.ts" )
  endforeach()

  add_custom_target(test_l10n
    ${LINGUIST_TS_ARGS}
  )

  set( LINGUIST_QM_ARGS COMMAND "${QT_LRELEASE_EXECUTABLE}" )

  set( generate_QMs )
  foreach(TESTS_LOCALE ${TESTS_LOCALES})
      set( generate_QMs ${generate_QMs} ${LINGUIST_QM_ARGS} "${CMAKE_CURRENT_SOURCE_DIR}/test_${TESTS_LOCALE}.ts" "-qm" "${CMAKE_CURRENT_BINARY_DIR}/test_${TESTS_LOCALE}.qm" )
  endforeach()

  add_custom_command(
    OUTPUT test_de_DE.qm test_fr_FR.qm
    ${generate_QMs}
    DEPENDS test_de_DE.ts test_fr_FR.ts
  )

  set(QM_FILES "
<qresource>
    <file alias=\"test_de_DE.qm\">${CMAKE_CURRENT_BINARY_DIR}/test_de_DE.qm</file>
    <file alias=\"test_fr_FR.qm\">${CMAKE_CURRENT_BINARY_DIR}/test_fr_FR.qm</file>
</qresource>")
endif()

configure_file(testresource.qrc.cmake "${CMAKE_CURRENT_BINARY_DIR}/testresource.qrc")

set (_rcc_file "${CMAKE_CURRENT_BINARY_DIR}/testresource.qrc")

qt6_add_resources(_testresource_rcc_src ${_rcc_file} OPTIONS -root "/" )

add_library(template_test_builtins STATIC
  ${_testresource_rcc_src}
)
target_link_libraries(template_test_builtins Qt6::Test)

macro(ktexttemplate_templates_unit_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname}_exec
                  ${_testname}.cpp
    )
    add_test(${_testname} ${CMAKE_BINARY_DIR}/bin/${_testname}_exec )
    target_link_libraries(${_testname}_exec KF6::TextTemplate template_test_builtins)

    if (Qt6Qml_FOUND)
      target_compile_definitions(${_testname}_exec PRIVATE HAVE_QTQML_LIB)
    endif()
  endforeach(_testname)
endmacro()

ktexttemplate_templates_unit_tests(
  testsafestring
  testbuiltins
  testloadertags
  testdefaulttags
  testcachingloader
  testfilters
  testgenerictypes
  testgenericcontainers
)

if (Qt6Qml_FOUND)
  ktexttemplate_templates_unit_tests(
    testscriptabletags
  )
endif()

if (Qt6LinguistTools_FOUND)
  ktexttemplate_templates_unit_tests(
    testinternationalization
  )
  if ("x${CMAKE_CXX_COMPILER_ID}x" MATCHES "xMSVCx")
    target_compile_options(testinternationalization_exec PRIVATE "/utf-8")
  endif()
endif()
