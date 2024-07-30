/* kgrantpty - helper program for KPty. */

/* This program is based on the glibc2.1 pt_chmod.
 * It was pulled out from there since both Linux
 * distributors and other OSes are not able to make
 * use of the glibc for different reasons.
 *
 * THIS IS A ROOT SUID PROGRAM
 *
 * Things work as following:
 *
 * In konsole we open a master pty. This can be opened
 * done by at most one process. Prior to opening the
 * master pty, the slave pty cannot be opened. Then, in
 * grantpty, we fork to this program. The trick is, that
 * the parameter is passes as a file handle, which cannot
 * be faked, so that we get a secure setuid root chmod/chown
 * with this program.
 *
 * We have to chown/chmod the slave pty to prevent eavesdroping.
 *
 * SPDX-FileCopyrightText: 1998 Zack Weinberg <zack@rabi.phys.columbia.edu>
 * SPDX-FileCopyrightText: 1999 Lars Doelle <lars.doelle@on-line.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <config-pty.h>

#include <cerrno>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#if HAVE_PTY_H
#include <pty.h>
#endif

#include <sys/param.h>
#if defined(__FreeBSD__)
#define BSD_PTY_HACK
#include <dirent.h>
#include <paths.h>
#endif

#define TTY_GROUP "tty"

int main(int argc, char *argv[])
{
    struct stat st;
    struct group *p;
    gid_t gid;
    uid_t uid;
    mode_t mod;
    char *tty;
    int fd;
#if !HAVE_PTSNAME && defined(TIOCGPTN)
    int ptyno;
    char ttyb[32];
#endif

    /* check preconditions **************************************************/
    if (argc != 3 || (strcmp(argv[1], "--grant") && strcmp(argv[1], "--revoke"))) {
        printf(
            "usage: %s (--grant|--revoke) <file descriptor>\n"
            "%s is a helper for the KDE core libraries.\n"
            "It is not intended to be called from the command line.\n"
            "It needs to be installed setuid root to function.\n",
            argv[0],
            argv[0]);
        return 1; /* FAIL */
    }

    if (geteuid() != 0) {
        fprintf(stderr, "%s not installed setuid root\n", argv[0]);
        return 1; /* FAIL */
    }

    fd = atoi(argv[2]);

    /* get slave pty name from master pty file handle *********/
#if HAVE_PTSNAME
    tty = ptsname(fd);
    if (!tty)
#elif defined(TIOCGPTN)
    if (!ioctl(fd, TIOCGPTN, &ptyno)) {
        sprintf(ttyb, "/dev/pts/%d", ptyno);
        tty = ttyb;
    } else
#endif
    {
        /* Check that fd is a valid master pseudo terminal.  */
        char *pty = ttyname(fd);

#ifdef BSD_PTY_HACK
        if (pty == NULL) {
            /*
              Hack to make kgrantpty work on some versions of FreeBSD (and possibly
              other systems): ttyname(3) does not work with a file descriptor opened
              on a /dev/pty?? device.

              Instead, this code looks through all the devices in /dev for a device
              which has the same inode as our PTY_FILENO descriptor... if found, we
              have the name for our pty.
            */

            struct dirent *dirp;
            DIR *dp;
            struct stat dsb;

            if (fstat(fd, &dsb) != -1) {
                if ((dp = opendir(_PATH_DEV)) != NULL) {
                    while ((dirp = readdir(dp))) {
                        if (dirp->d_fileno != dsb.st_ino) {
                            continue;
                        }
                        pty = malloc(sizeof(_PATH_DEV) + strlen(dirp->d_name));
                        if (pty) {
                            strcpy(pty, _PATH_DEV);
                            strcat(pty, dirp->d_name);
                        }
                        break;
                    }

                    (void)closedir(dp);
                }
            }
        }
#endif

        if (pty == NULL) {
            fprintf(stderr, "%s: cannot determine pty name.\n", argv[0]);
            return 1; /* FAIL */
        }

        /* matches /dev/pty?? */
        if (memcmp(pty, "/dev/pty", 8)) {
            fprintf(stderr, "%s: determined a strange pty name '%s'.\n", argv[0], pty);
            return 1; /* FAIL */
        }

        tty = malloc(strlen(pty) + 1);
        strcpy(tty, "/dev/tty");
        strcat(tty, pty + 8);
    }

    /* Check that the returned slave pseudo terminal is a character device.  */
    if (stat(tty, &st) < 0 || !S_ISCHR(st.st_mode)) {
        fprintf(stderr, "%s: found '%s' not to be a character device.\n", argv[0], tty);
        return 1; /* FAIL */
    }

    /* setup parameters for the operation ***********************************/

    if (!strcmp(argv[1], "--grant")) {
        uid = getuid();
        p = getgrnam(TTY_GROUP);
        if (!p) {
            p = getgrnam("wheel");
        }
        gid = p ? p->gr_gid : getgid();
        mod = S_IRUSR | S_IWUSR | S_IWGRP;
    } else {
        uid = 0;
        gid = st.st_gid == getgid() ? 0 : -1;
        mod = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    }

    /* Perform the actual chown/chmod ************************************/

    if (chown(tty, uid, gid) < 0) {
        fprintf(stderr, "%s: cannot chown %s: %s\n", argv[0], tty, strerror(errno));
        return 1; /* FAIL */
    }

    if (chmod(tty, mod) < 0) {
        fprintf(stderr, "%s: cannot chmod %s: %s\n", argv[0], tty, strerror(errno));
        return 1; /* FAIL */
    }

    return 0; /* OK */
}
