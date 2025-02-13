/***************************************************************************
**
** Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
** Copyright (c) 2012 - 2021 Jolla Ltd.
** Copyright (c) 2021 Open Mobile Platform LLC.
** All rights reserved.
** Contact: Nokia Corporation (directui@nokia.com)
**
** This file is part of applauncherd
**
** If you have questions regarding the use of this file, please contact
** Nokia at directui@nokia.com.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation
** and appearing in the file LICENSE.LGPL included in the packaging
** of this file.
**
****************************************************************************/

#define _GNU_SOURCE

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <limits.h>
#include <getopt.h>
#include <fcntl.h>
#include <time.h>
#include <poll.h>
#include <dbus/dbus.h>

#include "report.h"
#include "protocol.h"
#include "invokelib.h"
#include "search.h"

#define BOOSTER_SESSION "silica-session"
#define BOOSTER_GENERIC "generic"

/* Placeholder value used for regular boosters (that are not
 * sandboxed application boosters).
 */
#define UNDEFINED_APPLICATION "default"

/* Setting VERBOSE_SIGNALS to non-zero value logs receiving of
 * async-signals - which is useful only when actively debugging
 * booster / invoker interoperation.
 */
#define VERBOSE_SIGNALS 0

// Utility functions
static char *strip(char *str)
{
    if (str) {
        char *dst = str;
        char *src = str;
        while (*src && isspace(*src))
            ++src;
        for (;;) {
            while (*src && !isspace(*src))
                *dst++ = *src++;
            while (*src && isspace(*src))
                ++src;
            if (!*src)
                break;
            *dst++ = ' ';
        }
        *dst = 0;
    }
    return str;
}

static char *slice(const char *pos, const char **ppos, const char *delim)
{
    char *tok = NULL;
    if (pos) {
        const char *beg = pos;
        while (*pos && !strchr(delim, *pos))
            ++pos;
        tok = strndup(beg, pos - beg);
        if (*pos)
            ++pos;
    }
    if (ppos)
        *ppos = pos;
    return tok;
}

static char **split(const char *str, const char *delim)
{
    char **arr = NULL;
    if (str) {
        /* Upper limit for token count is number of delimeters + one */
        int n = 1;
        for (const char *pos = str; *pos; ++pos)
            if (strchr(delim, *pos))
                ++n;

        /* Upper limit for required array size is token count + one */
        arr = calloc(n + 1, sizeof *arr);

        /* Fill in the array */
        int i = 0;
        while (*str) {
            char *tok = slice(str, &str, delim);
            if (*strip(tok))
                arr[i++] = tok;
            else
                free(tok);
        }
        arr[i] = NULL;
    }
    return arr;
}

// Delay before exit.
static const unsigned int EXIT_DELAY     = 0;
static const unsigned int MIN_EXIT_DELAY = 1;
static const unsigned int MAX_EXIT_DELAY = 86400;

// Delay before a new booster is started. This will
// be sent to the launcher daemon.
static const unsigned int RESPAWN_DELAY     = 1;
static const unsigned int MIN_RESPAWN_DELAY = 0;
static const unsigned int MAX_RESPAWN_DELAY = 10;

static const unsigned char EXIT_STATUS_APPLICATION_NOT_FOUND = 0x7f;

// Environment
extern char ** environ;

// pid of the invoked process
static pid_t g_invoked_pid = -1;

static void sigs_restore(void);
static void sigs_init(void);

//! Pipe used to safely catch Unix signals
static int g_signal_pipe[2] = { -1, -1 };

// Forwards Unix signals from invoker to the invoked process
static void sig_forwarder(int sig)
{
#if VERBOSE_SIGNALS
    static const char m[] = "*** signal\n";
    if (write(STDERR_FILENO, m, sizeof m - 1) == -1) {
        // dontcare
    }
#endif

    // Write signal number to the self-pipe
    char signal_id = (char) sig;
    if (g_signal_pipe[1] == -1 || write(g_signal_pipe[1], &signal_id, 1) != 1) {
        const char m[] = "*** signal pipe write failure, terminating\n";
        if (write(STDERR_FILENO, m, sizeof m - 1) == -1) {
            // dontcare
        }
        _exit(EXIT_FAILURE);
    }
}

// Sets signal actions for Unix signals
static void sigs_set(struct sigaction *sig)
{
    sigaction(SIGINT,    sig, NULL);
    sigaction(SIGTERM,   sig, NULL);
}

// Sets up the signal forwarder
static void sigs_init(void)
{
    struct sigaction sig;

    memset(&sig, 0, sizeof(sig));
    sig.sa_flags = SA_RESTART;
    sig.sa_handler = sig_forwarder;

    sigs_set(&sig);
}

// Sets up the default signal handler
static void sigs_restore(void)
{
    struct sigaction sig;

    memset(&sig, 0, sizeof(sig));
    sig.sa_flags = SA_RESTART;
    sig.sa_handler = SIG_DFL;

    sigs_set(&sig);
}

static unsigned timestamp(void)
{
    struct timespec ts = { 0, 0 };
    if (clock_gettime(CLOCK_BOOTTIME, &ts) == -1 &&
        clock_gettime(CLOCK_MONOTONIC, &ts) == -1) {
        error("can't obtain a monotonic timestamp\n");
        exit(EXIT_FAILURE);
    }

    /* NOTE: caller must assume overflow to happen i.e.
     * the return values themselves mean nothing, only
     * differences between return values should be used.
     */
    return ((unsigned)(ts.tv_sec * 1000u) +
            (unsigned)(ts.tv_nsec / (1000 * 1000u)));
}

static bool shutdown_socket(int socket_fd)
{
    bool disconnected = false;

    /* Close transmit end from our side, then wait
     * for peer to receive EOF and close the receive
     * end too.
     */

    info("trying to disconnect booster socket...\n");

    if (shutdown(socket_fd, SHUT_WR) == -1) {
        warning("socket shutdown failed: %m\n");
        goto EXIT;
    }

    unsigned started = timestamp();
    unsigned timeout = 5000;
    for (;;) {
        unsigned elapsed = timestamp() - started;
        if (elapsed >= timeout)
            break;

        struct pollfd pfd = {
            .fd = socket_fd,
            .events = POLLIN,
            .revents = 0,
        };

        info("waiting for booster socket input...\n");
        int rc = poll(&pfd, 1, (int)(timeout - elapsed));

        if (rc == 0)
            break;

        if (rc == -1) {
            if (errno == EINTR || errno == EAGAIN)
                continue;
            warning("socket poll failed: %m\n");
            goto EXIT;
        }

        char buf[256];
        rc = recv(socket_fd, buf, sizeof buf, MSG_DONTWAIT);
        if (rc == 0) {
            /* EOF -> peer closed the socket */
            disconnected = true;
            goto EXIT;
        }

        if (rc == -1) {
            warning("socket read failed: %m\n");
            goto EXIT;
        }
    }
    warning("socket poll timeout\n");

EXIT:
    if (disconnected)
        info("booster socket was succesfully disconnected\n");
    else
        warning("could not disconnect booster socket\n");

    return disconnected;
}

static void kill_application(pid_t pid)
{
    if (pid == -1) {
        warning("application pid is not known, can't kill it");
        goto EXIT;
    }

    warning("sending SIGTERM to application (pid=%d)", (int)pid);

    if (kill(pid, SIGTERM) == -1)
        goto FAIL;

    for (int i = 0; i < 10; ++i) {
        sleep(1);
        if (kill(pid, 0) == -1)
            goto FAIL;
    }

    warning("sending SIGKILL to application (pid=%d)", (int)pid);

    if (kill(pid, SIGKILL) == -1)
        goto FAIL;

    for (int i = 0; i < 10; ++i) {
        sleep(1);
        if (kill(pid, 0) == -1)
            goto FAIL;
    }

    warning("application (pid=%d) did not exit", (int)pid);
    goto EXIT;

FAIL:
    if (errno == ESRCH)
        info("application (pid=%d) has exited", (int)pid);
    else
        warning("application (pid=%d) kill failed: %m", (int)pid);

EXIT:
    return;
}

// Receive ACK
static bool invoke_recv_ack(int fd)
{
    uint32_t action;

    invoke_recv_msg(fd, &action);

    if (action != INVOKER_MSG_ACK)
    {
        die(1, "Received wrong ack (%08x)\n", action);
    }

    return true;
}

// Inits a socket connection for the given application type
static int invoker_init(const char *app_type, const char *app_name)
{
    info("try type=%s app=%s ...", app_type, app_name);

    bool connected = false;
    int fd = -1;

    /* Sanity check args */
    if (!app_type || strchr(app_type, '/'))
        goto EXIT;
    if (app_name && strchr(app_name, '/'))
        goto EXIT;

    const char *runtimeDir = getenv("XDG_RUNTIME_DIR");
    if (!runtimeDir || !*runtimeDir) {
        error("XDG_RUNTIME_DIR is not defined.\n");
        goto EXIT;
    }

    if ((fd = socket(PF_UNIX, SOCK_STREAM, 0)) == -1) {
        error("Failed to create socket: %m\n");
        goto EXIT;
    }

    struct sockaddr_un sun = {
        .sun_family = AF_UNIX,
    };
    int maxSize = sizeof(sun.sun_path);
    int length;

    if (app_name)
        length = snprintf(sun.sun_path, maxSize, "%s/mapplauncherd/_%s/%s/socket",
                          runtimeDir, app_name, app_type);
    else
        length = snprintf(sun.sun_path, maxSize, "%s/mapplauncherd/%s",
                          runtimeDir, app_type);

    if (length <= 0 || length >= maxSize) {
        if (app_name)
            error("Invalid booster type: %s / application: %s\n",
                  app_type, app_name);
        else
            error("Invalid booster type: %s\n", app_type);
        goto EXIT;
    }

    if (connect(fd, (struct sockaddr *)&sun, sizeof(sun)) == -1) {
        if (errno != ENOENT)
            warning("connect(\"%s\") failed: %m\n", sun.sun_path);
        goto EXIT;
    }

    info("connected to: %s\n", sun.sun_path);
    connected = true;

EXIT:
    if (!connected && fd != -1)
        close(fd), fd = -1;
    return fd;
}

// Receives pid of the invoked process.
// Invoker doesn't know it, because the launcher daemon
// is the one who forks.
static uint32_t invoker_recv_pid(int fd)
{
    // Receive action.
    uint32_t action;
    invoke_recv_msg(fd, &action);
    if (action != INVOKER_MSG_PID)
        die(1, "Received a bad message id (%08x)\n", action);

    // Receive pid.
    uint32_t pid = 0;
    invoke_recv_msg(fd, &pid);
    if (pid == 0)
        die(1, "Received a zero pid \n");

    return pid;
}

// Receives exit status of the invoked process
static bool invoker_recv_exit(int fd, int* status)
{
    uint32_t action;

    // Receive action.
    bool res = invoke_recv_msg(fd, &action);

    if (!res || (action != INVOKER_MSG_EXIT))
    {
        // Boosted application process was killed somehow.
        // Let's give applauncherd process some time to cope 
        // with this situation.
        sleep(2);

        // If nothing happend, return
        return false;
    }
  
    // Receive exit status.
    res = invoke_recv_msg(fd, (uint32_t*) status);
    return res;
}

// Sends magic number / protocol version
static void invoker_send_magic(int fd, uint32_t options)
{
    // Send magic.
    invoke_send_msg(fd, INVOKER_MSG_MAGIC | INVOKER_MSG_MAGIC_VERSION | options);
}

// Sends the process name to be invoked.
static void invoker_send_name(int fd, const char *name)
{
    invoke_send_msg(fd, INVOKER_MSG_NAME);
    invoke_send_str(fd, name);
}

static void invoker_send_exec(int fd, char *exec)
{
    invoke_send_msg(fd, INVOKER_MSG_EXEC);
    invoke_send_str(fd, exec);
}

static void invoker_send_args(int fd, int argc, char **argv)
{
    int i;

    invoke_send_msg(fd, INVOKER_MSG_ARGS);
    invoke_send_msg(fd, argc);
    for (i = 0; i < argc; i++)
    {
        info("param %d %s \n", i, argv[i]);
        invoke_send_str(fd, argv[i]);
    }
}

static void invoker_send_prio(int fd, int prio)
{
    invoke_send_msg(fd, INVOKER_MSG_PRIO);
    invoke_send_msg(fd, prio);
}

// Sends booster respawn delay
static void invoker_send_delay(int fd, int delay)
{
    invoke_send_msg(fd, INVOKER_MSG_DELAY);
    invoke_send_msg(fd, delay);
}

// Sends UID and GID
static void invoker_send_ids(int fd, int uid, int gid)
{
    invoke_send_msg(fd, INVOKER_MSG_IDS);
    invoke_send_msg(fd, uid);
    invoke_send_msg(fd, gid);
}

// Sends the environment variables
static void invoker_send_env(int fd)
{
    int i, n_vars;

    // Count environment variables.
    for (n_vars = 0; environ[n_vars] != NULL; n_vars++) ;

    invoke_send_msg(fd, INVOKER_MSG_ENV);
    invoke_send_msg(fd, n_vars);

    for (i = 0; i < n_vars; i++)
    {
        invoke_send_str(fd, environ[i]);
    }

    return;
}

// Sends I/O descriptors
static void invoker_send_io(int fd)
{
    struct msghdr msg;
    struct cmsghdr *cmsg = NULL;
    int io[3] = { 0, 1, 2 };
    char buf[CMSG_SPACE(sizeof(io))];
    struct iovec iov;
    int dummy;

    memset(&msg, 0, sizeof(struct msghdr));

    iov.iov_base = &dummy;
    iov.iov_len = 1;

    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = buf;
    msg.msg_controllen = sizeof(buf);

    cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_len = CMSG_LEN(sizeof(io));
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;

    memcpy(CMSG_DATA(cmsg), io, sizeof(io));

    msg.msg_controllen = cmsg->cmsg_len;

    invoke_send_msg(fd, INVOKER_MSG_IO);
    if (sendmsg(fd, &msg, 0) < 0)
    {
        warning("sendmsg failed in invoker_send_io: %s \n", strerror(errno));
    }

    return;
}

// Sends the END message
static void invoker_send_end(int fd)
{
    invoke_send_msg(fd, INVOKER_MSG_END);
    invoke_recv_ack(fd);

}

// Prints the usage and exits with given status
static void usage(int status)
{
    printf("\n"
           "Usage: %s [options] [--type=TYPE] [file] [args]\n"
           "\n"
           "Launch applications compiled as a shared library (-shared) or\n"
           "a position independent executable (-pie) through mapplauncherd.\n"
           "\n"
           "TYPE chooses the type of booster used. Qt-booster may be used to\n"
           "launch anything. Possible values for TYPE:\n"
           "  qt5                    Launch a Qt 5 application.\n"
           "  qtquick2               Launch a Qt Quick 2 (QML) application.\n"
           "  lingmo               Launch a Lingmo application.\n"
           "  generic                Launch any application, even if it's not a library.\n"
           "\n"
           "The TYPE may also be a comma delimited list of boosters to try. The first available\n"
           "booster will be used.\n"
           "\n"
           "Options:\n"
           "  -t, --type TYPE        Define booster type\n"
           "  -a, --application APP  Define application booster name\n"
           "  -A, --auto-application Get application booster name from binary\n"
           "  -d, --delay SECS       After invoking sleep for SECS seconds\n"
           "                         (default %d).\n"
           "  -r, --respawn SECS     After invoking respawn new booster after SECS seconds\n"
           "                         (default %d, max %d).\n"
           "  -w, --wait-term        Wait for launched process to terminate (default).\n"
           "  -n, --no-wait          Do not wait for launched process to terminate.\n"
           "  -G, --global-syms      Places symbols in the application binary and its\n"
           "                         libraries to the global scope.\n"
           "                         See RTLD_GLOBAL in the dlopen manual page.\n"
           "  -D, --deep-syms        (TBD)"
           "  -s, --single-instance  Launch the application as a single instance.\n"
           "                         The existing application window will be activated\n"
           "                         if already launched.\n"
           "  -o, --keep-oom-score   Notify invoker that the launched process should inherit oom_score_adj\n"
           "                         from the booster. The score is reset to 0 normally.\n"
           "  -T, --test-mode        Invoker test mode. Also control file in root home should be in place.\n"
           "  -F, --desktop-file     Desktop file of the application to notify lipstick of launching app.\n"
           "  -I, --id               Sandboxing id to check if sandboxing should be forced.\n"
           "                         If this is not defined, it's guessed from binary name.\n"
           "  -h, --help             Print this help.\n"
           "  -v, --verbose          Make invoker more verbose. Can be given several times.\n"
           "\n"
           "Example: %s --type=lingmo /usr/bin/helloworld\n"
           "\n",
           PROG_NAME_INVOKER, EXIT_DELAY, RESPAWN_DELAY, MAX_RESPAWN_DELAY, PROG_NAME_INVOKER);

    exit(status);
}

// Return delay as integer 
static unsigned int get_delay(char *delay_arg, char *param_name,
                              unsigned int min_value, unsigned int max_value)
{
    unsigned int delay = EXIT_DELAY;

    if (delay_arg)
    {
        errno = 0; // To distinguish success/failure after call
        delay = strtoul(delay_arg, NULL, 10);

        // Check for various possible errors
        if ((errno == ERANGE && delay == ULONG_MAX)
            || delay < min_value
            || delay > max_value)
        {
            report(report_error, "Wrong value of %s parameter: %s\n", param_name, delay_arg);
            usage(1);
        }
    }
    
    return delay;
}

static int wait_for_launched_process_to_exit(int socket_fd)
{
    int exit_status = EXIT_FAILURE;
    int exit_signal = 0;

    // coverity[tainted_string_return_content]
    g_invoked_pid = invoker_recv_pid(socket_fd);
    info("Booster's pid is %d \n ", g_invoked_pid);

    // Setup signal handlers
    sigs_init();

    for (;;) {
        // Setup things for select()
        fd_set readfds;
        int ndfs = 0;

        FD_ZERO(&readfds);

        FD_SET(socket_fd, &readfds);
        ndfs = (socket_fd > ndfs) ? socket_fd : ndfs;

        // sig_forwarder() handles signals.
        // We only have to receive those here.
        FD_SET(g_signal_pipe[0], &readfds);
        ndfs = (g_signal_pipe[0] > ndfs) ? g_signal_pipe[0] : ndfs;

        // Wait for something appearing in the pipes.
        if (select(ndfs + 1, &readfds, NULL, NULL, NULL) == -1) {
            if (errno == EINTR || errno == EAGAIN)
                continue;
            warning("socket select failed: %m\n");
            break;
        }

        // Check if we got exit status from the invoked application
        if (FD_ISSET(socket_fd, &readfds)) {
            if (!invoker_recv_exit(socket_fd, &exit_status)) {
                // connection to application was lost
                exit_status = EXIT_FAILURE;
            } else {
                // there is no need to kill the application
                g_invoked_pid = -1;
            }
            break;
        }

        // Check if we got a UNIX signal.
        if (FD_ISSET(g_signal_pipe[0], &readfds)) {
            // Clean up the pipe
            char signal_id = 0;
            if (read(g_signal_pipe[0], &signal_id, 1) != 1) {
                error("signal pipe read failure, terminating\n");
                exit(EXIT_FAILURE);
            }
            exit_signal = signal_id;
            if (exit_signal == SIGTERM)
                exit_status = EXIT_SUCCESS;
            break;
        }
    }

    // Restore default signal handlers
    sigs_restore();

    if (exit_status != EXIT_SUCCESS)
        warning("application (pid=%d) exit(%d) signal(%d)\n",
                (int)g_invoked_pid, exit_status, exit_signal);
    else
        info("application (pid=%d) exit(%d) signal(%d)\n",
              (int)g_invoked_pid, exit_status, exit_signal);

    if (socket_fd != -1) {
        if (shutdown_socket(socket_fd))
            g_invoked_pid = -1;
        close(socket_fd),
            socket_fd = -1;
        if (g_invoked_pid != -1)
            kill_application(g_invoked_pid);
    }

    return exit_status;
}

typedef struct InvokeArgs {
    int           prog_argc;
    char        **prog_argv;
    char         *prog_name;
    const char   *app_type;
    const char   *app_name;
    uint32_t      magic_options;
    bool          wait_term;
    unsigned int  respawn_delay;
    bool          test_mode;
    const char   *desktop_file;
    char         *sandboxing_id;
    unsigned int  exit_delay;
} InvokeArgs;

#define INVOKE_ARGS_INIT {\
    .prog_argc     = 0,\
    .prog_argv     = NULL,\
    .prog_name     = NULL,\
    .app_type      = NULL,\
    .app_name      = UNDEFINED_APPLICATION,\
    .magic_options = INVOKER_MSG_MAGIC_OPTION_WAIT,\
    .wait_term     = true,\
    .respawn_delay = RESPAWN_DELAY,\
    .test_mode     = false,\
    .desktop_file  = NULL,\
    .sandboxing_id = NULL,\
    .exit_delay    = EXIT_DELAY,\
}

// "normal" invoke through a socket connection
static int invoke_remote(int socket_fd, const InvokeArgs *args)
{
    int exit_status = EXIT_FAILURE;

    // Get process priority
    errno = 0;
    int prog_prio = getpriority(PRIO_PROCESS, 0);
    if (errno && prog_prio < 0)
    {
        prog_prio = 0;
    }

    // Connection with launcher process is established,
    // send the data.
    invoker_send_magic(socket_fd, args->magic_options);
    invoker_send_name(socket_fd, args->prog_name);
    invoker_send_exec(socket_fd, args->prog_argv[0]);
    invoker_send_args(socket_fd, args->prog_argc, args->prog_argv);
    invoker_send_prio(socket_fd, prog_prio);
    invoker_send_delay(socket_fd, args->respawn_delay);
    invoker_send_ids(socket_fd, getuid(), getgid());
    invoker_send_io(socket_fd);
    invoker_send_env(socket_fd);
    invoker_send_end(socket_fd);

    if (args->wait_term) {
        exit_status = wait_for_launched_process_to_exit(socket_fd),
            socket_fd = -1;
    }

    if (socket_fd != -1)
        close(socket_fd);

    return exit_status;
}

static void invoke_fallback(const InvokeArgs *args)
{
    // Connection with launcher is broken,
    // try to launch application via execve
    warning("Connection with launcher process is broken. \n");
    error("Start application %s as a binary executable without launcher...\n", args->prog_name);

    // Fork if wait_term not set
    if (!args->wait_term)
    {
        // Fork a new process
        pid_t newPid = fork();

        if (newPid == -1)
        {
            error("Invoker failed to fork. \n");
            exit(EXIT_FAILURE);
        }
        else if (newPid != 0) /* parent process */
        {
            return;
        }
    }

    // Exec the process image
    execve(args->prog_name, args->prog_argv, environ);
    perror("execve");   /* execve() only returns on error */
    exit(EXIT_FAILURE);
}

// Invokes the given application
static int invoke(InvokeArgs *args)
{
    /* Note: Contents of 'args' are assumed to have been
     *       checked and sanitized before invoke() call.
     */

    int status = EXIT_FAILURE;

    /* The app can be launched with a comma delimited list of
     * booster types to attempt.
     */
    char **types = split(args->app_type, ",");

    int fd = -1;

    /* Session booster is a special case:
     * - is never going to be application specific
     * - can use and still uses legacy socket path
     * - mutually exclusive with all other choises
     * - no fallbacks should be utilized
     */

    bool tried_session = false;
    for (size_t i = 0; !tried_session && types[i]; ++i) {
        if (strcmp(types[i], BOOSTER_SESSION))
            continue;
        tried_session = true;
        fd = invoker_init(types[i], NULL);
    }

    /* Application aware boosters
     * - have fallback strategy, but it
     * - must not cross application vs UNDEFINED_APPLICATION boundary
     */
    if (fd == -1 && !tried_session) {
        bool tried_generic = false;
        for (size_t i = 0; fd == -1 && types[i]; ++i) {
            if (!strcmp(types[i], BOOSTER_GENERIC))
                tried_generic = true;
            fd = invoker_init(types[i], args->app_name);
        }
        if (fd == -1 && !tried_generic)
            fd = invoker_init(BOOSTER_GENERIC, args->app_name);
    }

    if (fd != -1) {
        /* "normal" invoke through a socket connetion */
        status = invoke_remote(fd, args),
            fd = -1;
    } else if (tried_session) {
        warning("Launch failed, session booster is not available.\n");
    } else if (strcmp(args->app_name, UNDEFINED_APPLICATION)) {
        /* Boosters that deal explicitly with one application only
         * must be assumed to run within sandbox -> skipping boosting
         * would also skip sandboxing -> no direct launch fallback
         */
        warning("Launch failed, application specific booster is not available.\n");
    } else {
        /* Give up and start unboosted */
        warning("Also fallback boosters failed, launch without boosting.\n");
        invoke_fallback(args);
        /* Returns only in case of: no-wait was specified and fork succeeded */
        status = EXIT_SUCCESS;
    }

    for (int i = 0; types[i]; ++i)
        free(types[i]);
    free(types);

    return status;
}

int main(int argc, char *argv[])
{
    InvokeArgs args = INVOKE_ARGS_INIT;
    bool auto_application = false;
    // Called with a different name (old way of using invoker) ?
    if (!strstr(argv[0], PROG_NAME_INVOKER) )
    {
        die(1,
            "Incorrect use of invoker, don't use symlinks. "
            "Run invoker explicitly from e.g. a D-Bus service file instead.\n");
    }

    // Options recognized
    struct option longopts[] = {
        {"help",             no_argument,       NULL, 'h'},
        {"wait-term",        no_argument,       NULL, 'w'},
        {"no-wait",          no_argument,       NULL, 'n'},
        {"global-syms",      no_argument,       NULL, 'G'},
        {"deep-syms",        no_argument,       NULL, 'D'},
        {"single-instance",  no_argument,       NULL, 's'},
        {"keep-oom-score",   no_argument,       NULL, 'o'},
        {"daemon-mode",      no_argument,       NULL, 'o'}, // Legacy alias
        {"test-mode",        no_argument,       NULL, 'T'},
        {"type",             required_argument, NULL, 't'},
        {"application",      required_argument, NULL, 'a'},
        {"auto-application", no_argument,       NULL, 'A'},
        {"delay",            required_argument, NULL, 'd'},
        {"respawn",          required_argument, NULL, 'r'},
        {"splash",           required_argument, NULL, 'S'}, // Legacy, ignored
        {"splash-landscape", required_argument, NULL, 'L'}, // Legacy, ignored
        {"desktop-file",     required_argument, NULL, 'F'},
        {"id",               required_argument, NULL, 'I'},
        {"verbose",          no_argument,       NULL, 'v'},
        {0, 0, 0, 0}
    };

    // Parse options
    // The use of + for POSIXLY_CORRECT behavior is a GNU extension, but avoids polluting
    // the environment
    int opt;
    while ((opt = getopt_long(argc, argv, "+hvcwnGDsoTd:t:a:Ar:S:L:F:I:", longopts, NULL)) != -1)
    {
        switch(opt)
        {
        case 'h':
            usage(0);
            break;

        case 'v':
            report_set_type(report_get_type() + 1);
            break;

        case 'w':
            // nothing to do, it's by default now
            break;

        case 'o':
            args.magic_options |= INVOKER_MSG_MAGIC_OPTION_OOM_ADJ_DISABLE;
            break;

        case 'n':
            args.wait_term = false;
            args.magic_options &= (~INVOKER_MSG_MAGIC_OPTION_WAIT);
            break;

        case 'G':
            args.magic_options |= INVOKER_MSG_MAGIC_OPTION_DLOPEN_GLOBAL;
            break;

        case 'D':
            args.magic_options |= INVOKER_MSG_MAGIC_OPTION_DLOPEN_DEEP;
            break;

        case 'T':
            args.test_mode = true;
            break;

        case 't':
            args.app_type = optarg;
            break;

        case 'a':
            args.app_name = optarg;
            auto_application = false;
            break;

        case 'A':
            auto_application = true;
            break;

        case 'd':
            args.exit_delay = get_delay(optarg, "delay", MIN_EXIT_DELAY, MAX_EXIT_DELAY);
            break;

        case 'r':
            args.respawn_delay = get_delay(optarg, "respawn delay",
                                      MIN_RESPAWN_DELAY, MAX_RESPAWN_DELAY);
            break;

        case 's':
            args.magic_options |= INVOKER_MSG_MAGIC_OPTION_SINGLE_INSTANCE;
            break;

        case 'S':
        case 'L':
            // Removed splash support. Ignore.
            break;

        case 'F':
            args.desktop_file = optarg;
            break;

        case 'I':
            args.sandboxing_id = strdup(optarg);
            break;

        case '?':
            usage(1);
        }
    }

    // Option processing stops as soon as application name is encountered

    args.prog_argc = argc - optind;
    args.prog_argv = &argv[optind];

    if (args.prog_argc < 1) {
        report(report_error, "No command line to invoke was given.\n");
        exit(EXIT_FAILURE);
    }

    // Force argv[0] of application to be the absolute path to allow the
    // application to find out its installation directory from there
    args.prog_argv[0] = search_program(args.prog_argv[0]);

    // Check if application exists
    struct stat file_stat;
    if (stat(args.prog_argv[0], &file_stat) == -1) {
        report(report_error, "%s: not found: %m\n", args.prog_argv[0]);
        return EXIT_STATUS_APPLICATION_NOT_FOUND;
    }

    // Check that application is regular file (or symlink to such)
    if (!S_ISREG(file_stat.st_mode)) {
        report(report_error, "%s: not a file\n", args.prog_argv[0]);
        return EXIT_STATUS_APPLICATION_NOT_FOUND;
    }

    // If it's a launcher, append its first argument to the name
    // (at this point, we have already checked if it exists and is a file)
    if (strcmp(args.prog_argv[0], "/usr/bin/sailfish-qml") == 0) {
        if (args.prog_argc < 2) {
            report(report_error, "%s: requires an argument\n", args.prog_argv[0]);
            return EXIT_STATUS_APPLICATION_NOT_FOUND;
        }

        if (asprintf(&args.prog_name, "%s %s", args.prog_argv[0], args.prog_argv[1]) < 0)
            exit(EXIT_FAILURE);
    } else {
        if (!(args.prog_name = strdup(args.prog_argv[0])))
            exit(EXIT_FAILURE);
    }

    if (auto_application)
        args.app_name = basename(args.prog_argv[0]);

    if (!args.app_type) {
        report(report_error, "Application type must be specified with --type.\n");
        usage(1);
    }

    if (!args.app_name) {
        report(report_error, "Application name must be specified with --application.\n");
        usage(1);
    }

    // If TEST_MODE_CONTROL_FILE doesn't exists switch off test mode
    if (args.test_mode && access(TEST_MODE_CONTROL_FILE, F_OK) != 0) {
        args.test_mode = false;
        info("Invoker test mode is not enabled.\n");
    }

    if (pipe(g_signal_pipe) == -1)
    {
        report(report_error, "Creating a pipe for Unix signals failed!\n");
        exit(EXIT_FAILURE);
    }

    // Send commands to the launcher daemon
    info("Invoking execution: '%s'\n", args.prog_name);
    int ret_val = invoke(&args);

    // Sleep for delay before exiting
    if (args.exit_delay) {
        // DBUS cannot cope some times if the invoker exits too early.
        info("Delaying exit for %d seconds..\n", args.exit_delay);
        sleep(args.exit_delay);
    }

    info("invoker exit(%d)\n", ret_val);
    return ret_val;
}
