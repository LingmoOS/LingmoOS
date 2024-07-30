find_program(PHPUNIT_EXECUTABLE phpunit HINTS ${CMAKE_SOURCE_DIR}/vendor/bin)
if (PHPUNIT_EXECUTABLE)
  set(PhpUnit_FOUND TRUE)
endif()
