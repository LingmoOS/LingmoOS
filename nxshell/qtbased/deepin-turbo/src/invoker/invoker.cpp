/***************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
#include <signal.h>
#include <sys/socket.h>
#include <bits/socket.h>
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

#include <cstring>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include "report.h"
#include "protocol.h"
#include "invokelib.h"
#include "search.h"

// Delay before exit.
static const unsigned int EXIT_DELAY     = 0;
static const unsigned int MIN_EXIT_DELAY = 1;
static const unsigned int MAX_EXIT_DELAY = 86400;

// Delay before a new booster is started. This will
// be sent to the launcher daemon.
static const unsigned int RESPAWN_DELAY     = 1;
static const unsigned int MIN_RESPAWN_DELAY = 0;
static const unsigned int MAX_RESPAWN_DELAY = 10;

static const unsigned char EXIT_STATUS_APPLICATION_CONNECTION_LOST = 0xfa;
static const unsigned char EXIT_STATUS_APPLICATION_NOT_FOUND = 0x7f;

// Environment
extern char ** environ;

// pid of the invoked process
static pid_t g_invoked_pid = -1;

static void sigs_restore(void);
static void sigs_init(void);

//! Pipe used to safely catch Unix signals
static int g_signal_pipe[2];

static const char *g_desktop_file = NULL;

// Forwards Unix signals from invoker to the invoked process
static void sig_forwarder(int sig)
{
    if (g_invoked_pid >= 0)
    {
        if (kill(g_invoked_pid, sig) != 0)
        {
            if (sig == SIGTERM && errno == ESRCH)
            {
                report(report_info,
                       "Can't send signal SIGTERM to application [%i] "
                       "because application is already terminated. \n",
                       g_invoked_pid);
            }
            else
            {
                report(report_error,
                      "Can't send signal %i to application [%i]: %s \n",
                      sig, g_invoked_pid, strerror(errno));
            }
        }

        // Restore signal handlers
        sigs_restore();

        // Write signal number to the self-pipe
        char signal_id = (char) sig;
        write(g_signal_pipe[1], &signal_id, 1);

        // Send the signal to itself using the default handler
        raise(sig);
#ifdef WITH_COVERAGE
        __gcov_flush();
#endif
    }
}

// Sets signal actions for Unix signals
static void sigs_set(struct sigaction *sig)
{
    sigaction(SIGABRT,   sig, NULL);
    sigaction(SIGALRM,   sig, NULL);
    sigaction(SIGBUS,    sig, NULL);
    sigaction(SIGCHLD,   sig, NULL);
    sigaction(SIGCONT,   sig, NULL);
    sigaction(SIGHUP,    sig, NULL);
    sigaction(SIGINT,    sig, NULL);
    sigaction(SIGIO,     sig, NULL);
    sigaction(SIGIOT,    sig, NULL);
    sigaction(SIGPIPE,   sig, NULL);
    sigaction(SIGPROF,   sig, NULL);
    sigaction(SIGPWR,    sig, NULL);
    sigaction(SIGQUIT,   sig, NULL);
    sigaction(SIGSEGV,   sig, NULL);
    sigaction(SIGSYS,    sig, NULL);
    sigaction(SIGTERM,   sig, NULL);
    sigaction(SIGTRAP,   sig, NULL);
    sigaction(SIGTSTP,   sig, NULL);
    sigaction(SIGTTIN,   sig, NULL);
    sigaction(SIGTTOU,   sig, NULL);
    sigaction(SIGUSR1,   sig, NULL);
    sigaction(SIGUSR2,   sig, NULL);
    sigaction(SIGVTALRM, sig, NULL);
    sigaction(SIGWINCH,  sig, NULL);
    sigaction(SIGXCPU,   sig, NULL);
    sigaction(SIGXFSZ,   sig, NULL);
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
static int invoker_init(const char *app_type)
{
    int fd;
    struct sockaddr_un sun;

    fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (fd < 0)
    {
        error("Failed to open invoker socket for type %s.\n", app_type);
        return -1;
    }

    sun.sun_family = AF_UNIX;
    int maxSize = sizeof(sun.sun_path) - 1;
 
    const char *runtimeDir = getenv("XDG_RUNTIME_DIR");
    const char *subpath = "/mapplauncherd/";
    const int subpathLen = strlen(subpath);

    if (runtimeDir && *runtimeDir)
        strncpy(sun.sun_path, runtimeDir, maxSize - subpathLen);
    else
        strncpy(sun.sun_path, "/tmp", maxSize - subpathLen);

    sun.sun_path[maxSize - subpathLen] = 0;
    strcat(sun.sun_path, subpath);

    maxSize -= strlen(sun.sun_path);
    if (maxSize < strlen(app_type) || strchr(app_type, '/'))
        die(1, "Invalid type of application: %s\n", app_type);

    strcat(sun.sun_path, app_type);

    if (connect(fd, (struct sockaddr *)&sun, sizeof(sun)) < 0)
    {
        error("Failed to initiate connect on the socket for type %s.\n", app_type);
        return -1;
    }

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
static void invoker_send_name(int fd, char *name)
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
        debug("param %d %s \n", i, argv[i]);
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
    printf("\nUsage: %s [options] [--type=TYPE] [file] [args]\n\n"
           "Launch applications compiled as a shared library (-shared) or\n"
           "a position independent executable (-pie) through mapplauncherd.\n\n"
           "TYPE chooses the type of booster used. Qt-booster may be used to\n"
           "launch anything. Possible values for TYPE:\n"
           "  dtkwidget              Launch a dtkwidget application.\n"
           "  generic                Launch any application, even if it's not a library.\n\n"
           "The TYPE may also be a comma delimited list of boosters to try. The first available\n"
           "booster will be used. If it is a desktop file, try get the type from desktop file"
           "'X-Deepin-TurboType' item.\n\n"
           "Options:\n"
           "  -d, --delay SECS       After invoking sleep for SECS seconds\n"
           "                         (default %d).\n"
           "  -r, --respawn SECS     After invoking respawn new booster after SECS seconds\n"
           "                         (default %d, max %d).\n"
           "  -w, --wait-term        Wait for launched process to terminate (default).\n"
           "  -n, --no-wait          Do not wait for launched process to terminate.\n"
           "  -G, --global-syms      Places symbols in the application binary and its\n"
           "                         libraries to the global scope.\n"
           "                         See RTLD_GLOBAL in the dlopen manual page.\n"
           "  -s, --single-instance  Launch the application as a single instance.\n"
           "                         The existing application window will be activated\n"
           "                         if already launched.\n"
           "  -o, --keep-oom-score   Notify invoker that the launched process should inherit oom_score_adj\n"
           "                         from the booster. The score is reset to 0 normally.\n"
           "  -T, --test-mode        Invoker test mode. Also control file in root home should be in place.\n"
           "  -F, --desktop-file     Desktop file of the application.\n"
           "  -h, --help             Print this help.\n\n"
           "Example: %s --type=dtkwidget /usr/bin/helloworld\n\n",
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

static int wait_for_launched_process_to_exit(int socket_fd, bool wait_term)
{
    int status = 0;

    // Wait for launched process to exit
    if (wait_term)
    {
        // coverity[tainted_string_return_content]
        g_invoked_pid = invoker_recv_pid(socket_fd);
        debug("Booster's pid is %d \n ", g_invoked_pid);

        // Forward UNIX signals to the invoked process
        sigs_init();

        while(1)
        {
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
            if (select(ndfs + 1, &readfds, NULL, NULL, NULL) > 0)
            {
                // Check if an exit status from the invoked application
                if (FD_ISSET(socket_fd, &readfds))
                {
                    bool res = invoker_recv_exit(socket_fd, &status);

                    if (!res)
                    {
                        // Because we are here, applauncherd.bin must be dead.
                        // Now we check if the invoked process is also dead
                        // and if not, we will kill it.
                        char filename[50];
                        snprintf(filename, sizeof(filename), "/proc/%d/cmdline", g_invoked_pid);

                        // Open filename for reading only
                        int fd = open(filename, O_RDONLY);
                        if (fd != -1)
                        {
                            // Application is still running
                            close(fd);

                            // Send a signal to kill the application too and exit.
                            // Sleep for some time to give
                            // the new applauncherd some time to load its boosters and
                            // the restart of g_invoked_pid succeeds.

                            sleep(10);
                            kill(g_invoked_pid, SIGKILL);
                            raise(SIGKILL);
                        }
                        else
                        {
                            // connection to application was lost
                            status = EXIT_FAILURE; 
                        }
                    }
                    break;
                }
                // Check if we got a UNIX signal.
                else if (FD_ISSET(g_signal_pipe[0], &readfds))
                {
                    // Clean up the pipe
                    char signal_id;
                    read(g_signal_pipe[0], &signal_id, sizeof(signal_id));

                    // Set signals forwarding to the invoked process again
                    // (they were reset by the signal forwarder).
                    sigs_init();
                }
            }
        }

        // Restore default signal handlers
        sigs_restore();
    }

    return status;
}

// "normal" invoke through a socket connection
static int invoke_remote(int socket_fd, int prog_argc, char **prog_argv, char *prog_name,
                         uint32_t magic_options, bool wait_term, unsigned int respawn_delay)
{
    // Get process priority
    errno = 0;
    int prog_prio = getpriority(PRIO_PROCESS, 0);
    if (errno && prog_prio < 0)
    {
        prog_prio = 0;
    }

    // Connection with launcher process is established,
    // send the data.
    invoker_send_magic(socket_fd, magic_options);
    invoker_send_name(socket_fd, prog_name);
    invoker_send_exec(socket_fd, prog_name);
    invoker_send_args(socket_fd, prog_argc, prog_argv);
    invoker_send_prio(socket_fd, prog_prio);
    invoker_send_delay(socket_fd, respawn_delay);
    invoker_send_ids(socket_fd, getuid(), getgid());
    invoker_send_io(socket_fd);
    invoker_send_env(socket_fd);
    invoker_send_end(socket_fd);

    if (prog_name)
    {
        free(prog_name);
    }

    int exit_status = wait_for_launched_process_to_exit(socket_fd, wait_term);
    return exit_status;
}

static void invoke_fallback(char **prog_argv, char *prog_name, bool wait_term)
{
    // Connection with launcher is broken,
    // try to launch application via execve
    warning("Connection with launcher process is broken. \n");
    error("Start application %s as a binary executable without launcher...\n", prog_name);

    // Fork if wait_term not set
    if(!wait_term)
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
    execve(prog_name, prog_argv, environ);
    perror("execve");   /* execve() only returns on error */
    exit(EXIT_FAILURE);
}

// Invokes the given application
static int invoke(int prog_argc, char **prog_argv, char *prog_name,
                  const char *app_type, uint32_t magic_options, bool wait_term, unsigned int respawn_delay,
                  bool test_mode)
{
    int status = 0;
    if (prog_name && prog_argv)
    {
        //If TEST_MODE_CONTROL_FILE doesn't exists switch off test mode
        if (test_mode && access(TEST_MODE_CONTROL_FILE, F_OK) != 0)
        {
            test_mode = false;
            info("Invoker test mode is not enabled.\n");
        }

        // The app can be launched with a comma delimited list of boosters to attempt.

        char *app_type_list = strdup(app_type);
        char *saveptr = NULL;
        char *token;

        int fd = -1;
        for (token = strtok_r(app_type_list, ",", &saveptr); fd == -1 && token != NULL; token = strtok_r(NULL, ",", &saveptr))
        {
            app_type = token;
            if (app_type != app_type_list) {
                info("Trying fallback type %s.\n", app_type);
            }
            fd = invoker_init(app_type);
        }

        if (fd == -1)
        {
            // This is a fallback if connection with the launcher
            // process is broken

            // if the attempt was to use the generic booster, and that failed,
            // then give up and start unboosted. otherwise, make an attempt to
            // use the generic booster before not boosting. this means single
            // instance support (for instance) will still work.
            if (strcmp(app_type, "generic") == 0)
            {
                warning("Can't try fall back to generic, already using it\n");
                invoke_fallback(prog_argv, prog_name, wait_term);
            }
            else
            {
                warning("Booster %s is not available. Falling back to generic.\n", app_type);
                invoke(prog_argc, prog_argv, prog_name, "generic", magic_options, wait_term, respawn_delay, test_mode);
            }
        }
        // "normal" invoke through a socket connetion
        else
        {
            status = invoke_remote(fd, prog_argc, prog_argv, prog_name,
                                   magic_options, wait_term, respawn_delay);
            close(fd);
        }

        free(app_type_list);
    }
    
    return status;
}

// 扩容二维数组，长度+10，复制数据到新数组
#define D_GROW_ARRAY2(u_size, pptr)  do {\
    char **tmp_arr = new char*[u_size + 10];\
    memcpy(tmp_arr, pptr, sizeof(pptr[0]) * u_size);\
    u_size += 10;\
    delete []pptr;\
    pptr = tmp_arr;\
}while(false)

int main(int argc, char *argv[])
{
    const char   *app_type      = NULL;
    std::string  turboType;
    int           prog_argc     = 0;
    uint32_t      magic_options = 0;
    bool          wait_term     = true;
    unsigned int  delay         = EXIT_DELAY;
    unsigned int  respawn_delay = RESPAWN_DELAY;
    char        **prog_argv     = NULL;
    char         *prog_name     = NULL;
    struct stat   file_stat;
    bool test_mode = false;

    // wait-term parameter by default
    magic_options |= INVOKER_MSG_MAGIC_OPTION_WAIT;

    // 用于支持binfmt调用直接启动desktop文件, 看文件是否是 .desktop 结尾
    const char *desktop_suffix = ".desktop";
    if (argc == 2 && strcmp(argv[1] + strlen(argv[1]) - strlen(desktop_suffix), desktop_suffix) == 0)
    {
        g_desktop_file = argv[1];
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
        {"delay",            required_argument, NULL, 'd'},
        {"respawn",          required_argument, NULL, 'r'},
        {"splash",           required_argument, NULL, 'S'},
        {"splash-landscape", required_argument, NULL, 'L'},
        {"desktop-file",     no_argument,       NULL, 'F'},
        {0, 0, 0, 0}
    };

    // Parse options
    // The use of + for POSIXLY_CORRECT behavior is a GNU extension, but avoids polluting
    // the environment
    int opt;
    while ((opt = getopt_long(argc, argv, "+hcwnGDsoTd:t:r:S:L:F:", longopts, NULL)) != -1)
    {
        switch(opt)
        {
        case 'h':
            usage(0);
            break;

        case 'w':
            // nothing to do, it's by default now
            break;

        case 'o':
            magic_options |= INVOKER_MSG_MAGIC_OPTION_OOM_ADJ_DISABLE;
            break;

        case 'n':
            wait_term = false;
            magic_options &= (~INVOKER_MSG_MAGIC_OPTION_WAIT);
            break;

        case 'G':
            magic_options |= INVOKER_MSG_MAGIC_OPTION_DLOPEN_GLOBAL;
            break;

        case 'D':
            magic_options |= INVOKER_MSG_MAGIC_OPTION_DLOPEN_DEEP;
            break;

        case 'T':
            test_mode = true;
            break;

        case 't':
            app_type = optarg;
            break;

        case 'd':
            delay = get_delay(optarg, "delay", MIN_EXIT_DELAY, MAX_EXIT_DELAY);
            break;

        case 'r':
            respawn_delay = get_delay(optarg, "respawn delay",
                                      MIN_RESPAWN_DELAY, MAX_RESPAWN_DELAY);
            break;

        case 's':
            magic_options |= INVOKER_MSG_MAGIC_OPTION_SINGLE_INSTANCE;
            break;

        case 'S':
        case 'L':
            // Removed splash support. Ignore.
            break;

        case 'F':
            g_desktop_file = argv[optind];
            break;

        case '?':
            usage(1);
        }
    }

    // Option processing stops as soon as application name is encountered
    if (optind < argc)
    {
        prog_argc = argc - optind;
        prog_argv = &argv[optind];

        if (g_desktop_file) {
            std::ifstream infile;
            infile.open(g_desktop_file, std::ios::binary);

            std::string line;
            std::string exec;

            while (std::getline(infile, line)) {
                if (std::strncmp(line.c_str(), "Exec=", 5) == 0) {
                    exec = std::move(line).substr(5);

                    // 如果不进行X-Deepin-TurboType字段的检测
                    if (app_type)
                        break;
                } else if (!app_type && std::strncmp(line.c_str(), "X-Deepin-TurboType=", 19) == 0) {
                    turboType = std::move(line).substr(19);
                }

                // 需要的数据都已经获取到时就返回
                if (!exec.empty() && !turboType.empty())
                    break;
            }

            // the progress arguments is contains option arg(start with "-")
            bool contains_command = false;
            for (int i = 1; i < prog_argc; ++i) {
                if (prog_argv[i][0] == '-') {
                    contains_command = true;
                    break;
                }
            }

            std::string progress;

            if (!contains_command) {
                uint psize = 10;
                uint new_pargc = 0;
                char **new_pargv = new char*[psize];

                std::istringstream iss(exec);
                iss >> progress;

                std::string tmp;
                while (std::getline(iss, tmp, ' ')) {
                    if (tmp.empty())
                        continue;

                    // ###(zccrs): process the '%' escape character
                    if (tmp[0] == '%' && tmp.size() == 2) {
                        switch (tmp[1]) {
                        case 'f': // ###(zccrs): ensure argument list is a single file
                        case 'F':
                        case 'u': // ###(zccrs): ensure argument list is a single url, and ensure locale file start width "file:://"
                        case 'U':
                            // replace placeholder
                            for (int i = 1; i < prog_argc; ++i) {
                                if (++new_pargc >= psize)
                                    D_GROW_ARRAY2(psize, new_pargv);

                                new_pargv[new_pargc] = prog_argv[i];
                            }
                            break;
                        // ###(zccrs): %i %c %k, from freedesktop desktop entry spec.
                        case 'i':
                        case 'c':
                        case 'k':
                        default:
                            break;
                        }
                    } else {
                        if (++new_pargc >= psize)
                            D_GROW_ARRAY2(psize, new_pargv);

                        const char *tmp_arg = tmp.c_str();
                        new_pargv[new_pargc] = new char[tmp.size() + 1];
                        strcpy(new_pargv[new_pargc], tmp_arg);
                    }
                }

                prog_argc = new_pargc + 1;
                prog_argv = new_pargv;
                // reset the desktop file path
                prog_argv[0] = argv[optind];
            } else {
                std::istringstream iss(exec);
                iss >> progress;
            }

            const char *dtkqml_booster = "dtkqml";
            bool is_dtkqml = (0 == std::strncmp(progress.c_str(), dtkqml_booster, std::strlen(dtkqml_booster)));
            if (is_dtkqml) {
                prog_name = strdup(progress.c_str());
                if (!prog_name)
                {
                    die(1, "allocating prog_name buffer");
                }
            } else {
                prog_name = search_program(progress.c_str());
            }
        } else {
            prog_name = search_program(argv[optind]);
            // Force argv[0] of application to be the absolute path to allow the
            // application to find out its installation directory from there
            prog_argv[0] = prog_name;
        }
    }

    // Check if application name isn't defined
    if (!prog_name)
    {
        report(report_error, "Application's name is not defined.\n");
        usage(1);
    }

    // Check if application exists
    if (stat(prog_name, &file_stat))
    {
        report(report_error, "%s: not found\n", prog_name);
        return EXIT_STATUS_APPLICATION_NOT_FOUND;
    }

    // Check that 
    if (!S_ISREG(file_stat.st_mode) && !S_ISLNK(file_stat.st_mode))
    {
        report(report_error, "%s: not a file\n", prog_name);
        return EXIT_STATUS_APPLICATION_NOT_FOUND;
    }

    // If it's a launcher, append its first argument to the name
    // (at this point, we have already checked if it exists and is a file)
    if (strcmp(prog_name, "/usr/bin/sailfish-qml") == 0) {
        if (prog_argc < 2) {
            report(report_error, "%s: requires an argument\n", prog_name);
            return EXIT_STATUS_APPLICATION_NOT_FOUND;
        }

        // Must not free() the existing prog_name, as it's pointing to prog_argv[0]
        prog_name = (char *)malloc(strlen(prog_argv[0]) + strlen(prog_argv[1]) + 2);
        sprintf(prog_name, "%s %s", prog_argv[0], prog_argv[1]);
    }

    if (!app_type && !turboType.empty()) {
        // 使用从desktop文件获取的应用类型
        app_type = turboType.c_str();
    }

    if (!app_type)
    {
        report(report_error, "Application type must be specified with --type.\n");
        usage(1);
    }

    if (pipe(g_signal_pipe) == -1)
    { 
        report(report_error, "Creating a pipe for Unix signals failed!\n"); 
        exit(EXIT_FAILURE); 
    }

    // Send commands to the launcher daemon
    info("Invoking execution: '%s'\n", prog_name);
    int ret_val = invoke(prog_argc, prog_argv, prog_name, app_type, magic_options, wait_term, respawn_delay, test_mode);

    // Sleep for delay before exiting
    if (delay)
    {
        // DBUS cannot cope some times if the invoker exits too early.
        debug("Delaying exit for %d seconds..\n", delay);
        sleep(delay);
    }

    return ret_val;
}
