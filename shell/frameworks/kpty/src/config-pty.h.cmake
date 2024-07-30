/* Defined to the path of the PTY multiplexer device, if any */
#cmakedefine PTM_DEVICE "${PTM_DEVICE}"

#cmakedefine01 HAVE_POSIX_OPENPT
#cmakedefine01 HAVE_GETPT 
#cmakedefine01 HAVE_GRANTPT
#cmakedefine01 HAVE_OPENPTY
#cmakedefine01 HAVE_PTSNAME
#cmakedefine01 HAVE_REVOKE
#cmakedefine01 HAVE_UNLOCKPT
#cmakedefine01 HAVE__GETPTY
#cmakedefine01 HAVE_TCGETATTR
#cmakedefine01 HAVE_TCSETATTR

#cmakedefine01 HAVE_LIBUTIL_H
#cmakedefine01 HAVE_UTIL_H
#cmakedefine01 HAVE_PTY_H
/* Unused ? */
#cmakedefine01 HAVE_TERMIOS_H
#cmakedefine01 HAVE_TERMIO_H
#cmakedefine01 HAVE_SYS_STROPTS_H
#cmakedefine01 HAVE_SYS_FILIO_H

#cmakedefine01 HAVE_UTEMPTER
#cmakedefine01 HAVE_LOGIN
#cmakedefine01 HAVE_UTMPX
#cmakedefine01 HAVE_LOGINX
#cmakedefine01 HAVE_STRUCT_UTMP_UT_TYPE
#cmakedefine01 HAVE_STRUCT_UTMP_UT_PID
#cmakedefine01 HAVE_STRUCT_UTMP_UT_SESSION
#cmakedefine01 HAVE_STRUCT_UTMP_UT_SYSLEN
#cmakedefine01 HAVE_STRUCT_UTMP_UT_ID

#cmakedefine01 HAVE_SYS_TIME_H

/*
 * Steven Schultz <sms at to.gd-es.com> tells us :
 * BSD/OS 4.2 doesn't have a prototype for openpty in its system header files
 */
#ifdef __bsdi__
__BEGIN_DECLS
int openpty(int *, int *, char *, struct termios *, struct winsize *);
__END_DECLS
#endif

#define CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}"
#define KDE_INSTALL_LIBEXECDIR_KF "${KDE_INSTALL_LIBEXECDIR_KF}"
