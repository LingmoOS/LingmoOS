/*
    kdesu_stub.c: KDE su executes this stub through su or ssh. This stub in turn
                  executes the target program. Before that, startup parameters
                  are sent through stdin.

    This file is part of the KDE project, module kdesu.
    SPDX-FileCopyrightText: 1999, 2000 Geert Jansen <jansen@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later


    Available parameters:

      Parameter       Description         Format (csl = comma separated list)

    - kdesu_stub      Header              "ok" | "stop"
    - display         X11 display         string
    - display_auth    X11 authentication  "type cookie" pair
    - command         Command to run      string
    - path            PATH env. var       string
    - build_sycoca    Rebuild sycoca?     "yes" | "no"
    - user            Target user         string
    - priority        Process priority    0 <= int <= 100
    - scheduler       Process scheduler   "fifo" | "normal"
    - app_startup_id  DESKTOP_STARTUP_ID  string
    - environment     Additional envvars  strings, last one is empty
*/

#include <config-kdesu.h>

#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#if HAVE_INITGROUPS
#include <grp.h>
#endif

#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#ifdef POSIX1B_SCHEDULING
#include <sched.h>
#endif

/**
 * Params sent by the peer.
 */

struct param_struct {
    const char *name;
    char *value;
};

struct param_struct params[] = {{"kdesu_stub", 0L},
                                {"display", 0L},
                                {"display_auth", 0L},
                                {"command", 0L},
                                {"path", 0L},
                                {"xwindows_only", 0L},
                                {"user", 0L},
                                {"priority", 0L},
                                {"scheduler", 0L},
                                /* obsoleted by app_startup_id    { "app_start_pid", 0L } */
                                {"app_startup_id", 0L}};

#define P_HEADER 0
#define P_DISPLAY 1
#define P_DISPLAY_AUTH 2
#define P_COMMAND 3
#define P_PATH 4
#define P_XWIN_ONLY 5
#define P_USER 6
#define P_PRIORITY 7
#define P_SCHEDULER 8
#define P_APP_STARTUP_ID 9
#define P_LAST 10

/**
 * Safe malloc functions.
 */
char *xmalloc(size_t size)
{
    char *ptr = malloc(size);
    if (ptr) {
        return ptr;
    }
    perror("malloc()");
    exit(1);
}

char **xrealloc(char **ptr, int size)
{
    ptr = realloc(ptr, size);
    if (ptr) {
        return ptr;
    }
    perror("realloc()");
    exit(1);
}

/**
 * Solaris does not have a setenv()...
 */
int xsetenv(const char *name, const char *value)
{
    char *s = malloc(strlen(name) + strlen(value) + 2);
    if (!s) {
        return -1;
    }
    strcpy(s, name);
    strcat(s, "=");
    strcat(s, value);
    return putenv(s); /* yes: no free()! */
}

/**
 * Safe strdup and strip newline
 */
char *xstrdup(char *src)
{
    int len = strlen(src);
    char *dst = xmalloc(len + 1);
    strcpy(dst, src);
    if (dst[len - 1] == '\n') {
        dst[len - 1] = '\000';
    }
    return dst;
}

/**
 * Split comma separated list.
 */
char **xstrsep(char *str)
{
    int i = 0;
    int size = 10;
    char **list = (char **)xmalloc(size * sizeof(char *));
    char *ptr = str;
    char *nptr;
    while ((nptr = strchr(ptr, ',')) != 0L) {
        if (i > size - 2) {
            list = xrealloc(list, (size *= 2) * sizeof(char *));
        }
        *nptr = '\000';
        list[i++] = ptr;
        ptr = nptr + 1;
    }
    if (*ptr != '\000') {
        list[i++] = ptr;
    }
    list[i] = 0L;
    return list;
}

#define BUFSIZE 8192

static void dequote(char *buf)
{
    char *in;
    char *out;
    for (in = buf, out = buf; *in; in++, out++) {
        char c = *in;
        if (c == '\\') {
            c = *++in;
            if (c == '/') {
                *out = '\\';
            } else {
                *out = c - '@';
            }
        } else {
            *out = c;
        }
    }
    *out = 0;
}

/**
 * The main program
 */

int main()
{
    char buf[BUFSIZE + 1];
    char xauthority[200];
    int i;
    int prio;
    pid_t pid;
    FILE *fout;
    struct passwd *pw;
    const char *kdesu_lc_all;

    xauthority[0] = '\0';

    /* Get startup parameters. */

    for (i = 0; i < P_LAST; i++) {
        printf("%s\n", params[i].name);
        fflush(stdout);
        if (fgets(buf, BUFSIZE, stdin) == 0L) {
            printf("end\n");
            fflush(stdout);
            perror("kdesu_stub: fgets()");
            exit(1);
        }
        params[i].value = xstrdup(buf);
        /* Installation check? */
        if (i == 0 && !strcmp(params[i].value, "stop")) {
            printf("end\n");
            exit(0);
        }
    }
    printf("environment\n");
    fflush(stdout);
    for (;;) {
        char *tmp;
        if (fgets(buf, BUFSIZE, stdin) == 0L) {
            printf("end\n");
            fflush(stdout);
            perror("kdesu_stub: fgets()");
            exit(1);
        }
        dequote(buf);
        tmp = xstrdup(buf);
        if (tmp[0] == '\0') { /* terminator */
            break;
        }
        putenv(tmp);
    }

    printf("end\n");
    fflush(stdout);

    xsetenv("PATH", params[P_PATH].value);
    xsetenv("DESKTOP_STARTUP_ID", params[P_APP_STARTUP_ID].value);

    kdesu_lc_all = getenv("KDESU_LC_ALL");
    if (kdesu_lc_all != NULL) {
        xsetenv("LC_ALL", kdesu_lc_all);
    } else {
        unsetenv("LC_ALL");
    }

    unsetenv("XDG_RUNTIME_DIR");

    /* Do we need to change uid? */

    pw = getpwnam(params[P_USER].value);
    if (pw == 0L) {
        printf("kdesu_stub: user %s does not exist!\n", params[P_USER].value);
        exit(1);
    }
    xsetenv("HOME", pw->pw_dir);

    /* New user won't be able to connect it anyway. */
    unsetenv("DBUS_SESSION_BUS_ADDRESS");

    /* Set scheduling/priority */

    prio = atoi(params[P_PRIORITY].value);
    if (!strcmp(params[P_SCHEDULER].value, "realtime")) {
#ifdef POSIX1B_SCHEDULING
        struct sched_param sched;
        int min = sched_get_priority_min(SCHED_FIFO);
        int max = sched_get_priority_max(SCHED_FIFO);
        sched.sched_priority = min + (int)(((double)prio) * (max - min) / 100 + 0.5);
        sched_setscheduler(0, SCHED_FIFO, &sched);
#else
        printf("kdesu_stub: realtime scheduling not supported\n");
#endif
    } else {
#if HAVE_SETPRIORITY
        int val = 20 - (int)(((double)prio) * 40 / 100 + 0.5);
        setpriority(PRIO_PROCESS, getpid(), val);
#endif
    }

    /* Drop privileges (this is permanent) */

    if (getuid() != pw->pw_uid) {
        if (setgid(pw->pw_gid) == -1) {
            perror("kdesu_stub: setgid()");
            exit(1);
        }
#if HAVE_INITGROUPS
        if (initgroups(pw->pw_name, pw->pw_gid) == -1) {
            perror("kdesu_stub: initgroups()");
            exit(1);
        }
#endif
        if (setuid(pw->pw_uid) == -1) {
            perror("kdesu_stub: setuid()");
            exit(1);
        }
        xsetenv("HOME", pw->pw_dir);
    }

    /* Handle display */

    if (strcmp(params[P_DISPLAY].value, "no")) {
        xsetenv("DISPLAY", params[P_DISPLAY].value);
        if (params[P_DISPLAY_AUTH].value[0]) {
            int fd2;
            /*
             ** save umask and set to 077, so we create those files only
             ** readable for root. (if someone else could read them, we
             ** are in deep shit).
             */
            int oldumask = umask(077);
            const char *disp = params[P_DISPLAY].value;
            if (strncmp(disp, "localhost:", 10) == 0) {
                disp += 9;
            }

            strcpy(xauthority, "/tmp/xauth.XXXXXXXXXX");
            fd2 = mkstemp(xauthority);
            umask(oldumask);

            if (fd2 == -1) {
                perror("kdesu_stub: mkstemp()");
                exit(1);
            } else {
                close(fd2);
            }
            xsetenv("XAUTHORITY", xauthority);

            fout = popen("xauth >/dev/null 2>&1", "w");
            if (fout == NULL) {
                perror("kdesu_stub: popen(xauth)");
                exit(1);
            }
            fprintf(fout, "add %s %s\n", disp, params[P_DISPLAY_AUTH].value);
            pclose(fout);
        }
    }

    /* Rebuild the sycoca and start kdeinit? */

    if (strcmp(params[P_XWIN_ONLY].value, "no")) {
        system("kdeinit5 --suicide");
    }

    /* Execute the command */

    pid = fork();
    if (pid == -1) {
        perror("kdesu_stub: fork()");
        exit(1);
    }
    if (pid) {
        /* Parent: wait for child, delete tempfiles and return. */
        int ret;
        int state;
        int xit = 1;
        while (1) {
            ret = waitpid(pid, &state, 0);
            if (ret == -1) {
                if (errno == EINTR) {
                    continue;
                }
                if (errno != ECHILD) {
                    perror("kdesu_stub: waitpid()");
                }
                break;
            }
            if (WIFEXITED(state)) {
                xit = WEXITSTATUS(state);
            }
        }

        if (*xauthority) {
            unlink(xauthority);
        }
        exit(xit);
    } else {
        setsid();
        /* Child: exec command. */
        sprintf(buf, "%s", params[P_COMMAND].value);
        dequote(buf);
        execl("/bin/sh", "sh", "-c", buf, (void *)0);
        perror("kdesu_stub: exec()");
        _exit(1);
    }
}
