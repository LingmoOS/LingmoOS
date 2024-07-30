include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckCXXSymbolExists)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckStructHasMember)

check_include_files(sys/time.h    HAVE_SYS_TIME_H)

if (UNIX)
  check_include_files("sys/types.h;libutil.h" HAVE_LIBUTIL_H)
  check_include_files(util.h       HAVE_UTIL_H)
  check_include_files(termios.h    HAVE_TERMIOS_H)
  check_include_files(termio.h     HAVE_TERMIO_H)
  check_include_files(pty.h        HAVE_PTY_H)
  check_include_files(sys/stropts.h HAVE_SYS_STROPTS_H)
  check_include_files(sys/filio.h  HAVE_SYS_FILIO_H)

  set(UTIL_LIBRARY)

  if (NOT UTEMPTER_FOUND)
    check_function_exists(login login_in_libc)
    if (NOT login_in_libc)
      check_library_exists(util login "" login_in_libutil)
      if (login_in_libutil)
        set(UTIL_LIBRARY util)
      endif (login_in_libutil)
    endif (NOT login_in_libc)
    if (CMAKE_SYSTEM_NAME MATCHES Linux OR CMAKE_SYSTEM_NAME MATCHES Darwin OR CMAKE_SYSTEM_NAME MATCHES kFreeBSD OR CMAKE_SYSTEM_NAME STREQUAL GNU)
      set (HAVE_UTMPX)
    else (CMAKE_SYSTEM_NAME MATCHES Linux OR CMAKE_SYSTEM_NAME MATCHES Darwin OR CMAKE_SYSTEM_NAME MATCHES kFreeBSD OR CMAKE_SYSTEM_NAME STREQUAL GNU)
      check_function_exists(getutxent HAVE_UTMPX)
    endif (CMAKE_SYSTEM_NAME MATCHES Linux OR CMAKE_SYSTEM_NAME MATCHES Darwin OR CMAKE_SYSTEM_NAME MATCHES kFreeBSD OR CMAKE_SYSTEM_NAME STREQUAL GNU)
    if (HAVE_UTMPX)
      set(utmp utmpx)
      if (login_in_libutil)
        check_library_exists(util loginx "" HAVE_LOGINX)
      endif (login_in_libutil)
    else (HAVE_UTMPX)
      set(utmp utmp)
    endif (HAVE_UTMPX)
    if (login_in_libc OR login_in_libutil)
      set(HAVE_LOGIN 1)
    else (login_in_libc OR login_in_libutil)
      set(HAVE_LOGIN)
      check_struct_has_member("struct ${utmp}" "ut_type" "${utmp}.h" HAVE_STRUCT_UTMP_UT_TYPE)
      check_struct_has_member("struct ${utmp}" "ut_pid" "${utmp}.h" HAVE_STRUCT_UTMP_UT_PID)
      check_struct_has_member("struct ${utmp}" "ut_session" "${utmp}.h" HAVE_STRUCT_UTMP_UT_SESSION)
    endif (login_in_libc OR login_in_libutil)
    check_struct_has_member("struct ${utmp}" "ut_syslen" "${utmp}.h" HAVE_STRUCT_UTMP_UT_SYSLEN)
    check_struct_has_member("struct ${utmp}" "ut_id" "${utmp}.h" HAVE_STRUCT_UTMP_UT_ID)
  endif (NOT UTEMPTER_FOUND)

  check_function_exists(openpty openpty_in_libc)
  if (NOT openpty_in_libc)
    check_library_exists(util openpty "" openpty_in_libutil)
    if (openpty_in_libutil)
      set(UTIL_LIBRARY util)
    endif (openpty_in_libutil)
  endif (NOT openpty_in_libc)
  if (openpty_in_libc OR openpty_in_libutil)
    set(HAVE_OPENPTY 1)
  else (openpty_in_libc OR openpty_in_libutil)
    set(HAVE_OPENPTY)

    execute_process(
      COMMAND sh -c "
        for ptm in ptc ptmx ptm ptym/clone; do
          if test -c /dev/$ptm; then
            echo /dev/$ptm
            break
          fi
        done"
      OUTPUT_VARIABLE PTM_DEVICE
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    message(STATUS "PTY multiplexer: ${PTM_DEVICE}")

    check_function_exists(revoke     HAVE_REVOKE)
    check_function_exists(_getpty    HAVE__GETPTY)
    check_function_exists(getpt      HAVE_GETPT)
    check_function_exists(grantpt    HAVE_GRANTPT)
    check_function_exists(unlockpt   HAVE_UNLOCKPT)
    check_function_exists(posix_openpt HAVE_POSIX_OPENPT)
  endif (openpty_in_libc OR openpty_in_libutil)

  check_function_exists(ptsname    HAVE_PTSNAME)
  check_function_exists(tcgetattr  HAVE_TCGETATTR)
  check_function_exists(tcsetattr  HAVE_TCSETATTR)
endif (UNIX)

