include(ECMMarkAsTest)

find_package(Qt6Test ${REQUIRED_QT_VERSION} CONFIG REQUIRED)

# needed by loadcalendar
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/cal DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

macro(macro_unit_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    add_test(NAME ${_testname} COMMAND ${_testname})
    ecm_mark_as_test(${_testname})
    target_link_libraries(${_testname} KF6CalendarCore Qt6::Test LibIcal)
  endforeach()
endmacro()

macro_unit_tests(
  testalarm
  testattachment
  testattendee
  testcalfilter
  testconference
  testcustomproperties
  testdateserialization
  testduration
  testevent
  testincidence
  testexception
  testfilestorage
  testfreebusy
  testincidencerelation
  testicalformat
  testidentical
  testjournal
  testmemorycalendar
  testperiod
  testfreebusyperiod
  testperson
  testrecurtodo
  teststartdatetimesfordate
  teststatus
  testtodo
  testtimesininterval
  testcreateddatecompat
  testrecurrenceexception
  testoccurrenceiterator
  testreadrecurrenceid
  incidencestest
  loadcalendar
  fbrecurring
  readandwrite
  testfb
  testrecurprevious
  testrecurrence
  testrecurrencetype
  testrecurson
  testtostring
  testvcalexport
  testcalendarobserver
  calendarlistmodeltest
)

set_target_properties(testmemorycalendar PROPERTIES COMPILE_FLAGS -DICALTESTDATADIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/data/\\"")
set_target_properties(testreadrecurrenceid PROPERTIES COMPILE_FLAGS -DICALTESTDATADIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/data/\\"")
set_target_properties(testconference PROPERTIES COMPILE_FLAGS -DICALTESTDATADIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/data/\\"")
# this test cannot work with msvc because libical should not be altered
# and therefore we can't add KCALENDARCORE_EXPORT there
# it should work fine with mingw because of the auto-import feature
# TODO: check if this can build with msvc now (and if it works on Windows)
if(NOT MSVC)
  macro_unit_tests(testicaltimezones)
  target_link_libraries(testicaltimezones Qt6::Core KF6::CalendarCore)
endif()

########### Tests #######################

file(GLOB_RECURSE testFiles data/RecurrenceRule/*.ics)
file(GLOB_RECURSE vCalFiles data/vCalendar/*.vcs)
file(GLOB_RECURSE compatFiles data/Compat/*.ics)

find_program(PERL_EXECUTABLE perl)

macro(kcalcore_run_single_test _prefix _f _test _testarg)
  get_filename_component(_fn ${_f} NAME)

  # Write the output file in the build directory.
  string(REGEX REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}" _outFile ${_f})
  get_filename_component(_outputPath ${_outFile} PATH)
  file(MAKE_DIRECTORY ${_outputPath})

  set(_runner ${PERL_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/runsingletestcase.pl)
  add_test(NAME ${_prefix}-${_fn} COMMAND ${_runner} $<TARGET_FILE:${_test}> ${_testarg} ${_f} ${_outFile})
endmacro()

file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/FAILED.log)

foreach(file ${testFiles})
  kcalcore_run_single_test(RecurNext ${file} testrecurrence "next")
endforeach()

foreach(file ${testFiles})
  kcalcore_run_single_test(RecurPrev ${file} testrecurprevious "prev")
endforeach()

foreach(file ${testFiles})
  kcalcore_run_single_test(RecursOn ${file} testrecurson "recurson")
endforeach()

foreach(file ${compatFiles})
  kcalcore_run_single_test(Compat ${file} readandwrite "ical")
endforeach()

foreach(file ${vCalFiles})
  kcalcore_run_single_test(VCalIn ${file} readandwrite "ical")
endforeach()
