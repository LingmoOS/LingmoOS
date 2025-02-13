/***************************************************************************
**
** Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
** Copyright (c) 2013 - 2021 Jolla Ltd.
** Copyright (c) 2020 - 2021 Open Mobile Platform LLC.
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

#include "daemon.h"
#include "logger.h"
#include "report.h"
#include "connection.h"
#include "booster.h"
#include "singleinstance.h"
#include "socketmanager.h"

#include <deque>
#include <cstdlib>
#include <cerrno>
#include <sys/capability.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <glob.h>
#include <cstring>
#include <cstdio>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <libgen.h>
#include <stdlib.h>
#include <systemd/sd-daemon.h>
#include <unistd.h>
#include <poll.h>
#include <getopt.h>

#include "coverage.h"

/* Setting VERBOSE_SIGNALS to non-zero value logs receiving of
 * async-signals - which is useful only when actively debugging
 * booster / invoker interoperation.
 */
#define VERBOSE_SIGNALS 01


// Environment
extern char ** environ;

Daemon * Daemon::m_instance = NULL;
const int Daemon::m_boosterSleepTime = 2;

static void write_dontcare(int fd, const void *data, size_t size)
{
    ssize_t rc = write(fd, data, size);
    if (rc == -1)
        Logger::logWarning("write to fd=%d failed: %m", fd);
    else if ((size_t)rc != size)
        Logger::logWarning("write to fd=%d failed", fd);
}

#if VERBOSE_SIGNALS
static char *addstr(char *pos, char *end, const char *str)
{
    while (pos < end && *str)
        *pos++ = *str++;
    return pos;
}
static char *addnum(char *pos, char *end, int num)
{
    char stk[64];
    int sp = sizeof stk;
    bool neg = (num < 0);
    unsigned mag = (unsigned)(neg ? -num : num);
    stk[--sp] = 0;
    do {
        stk[--sp] = '0' + mag % 10;
    } while (mag /= 10);
    if (neg)
        stk[--sp] = '-';
    return addstr(pos, end, &stk[sp]);
}
#endif

static void write_to_signal_pipe(int sig)
{
#if VERBOSE_SIGNALS
    char m[256];
    char *p = m;
    char *e = m + sizeof m - 1;
    p = addstr(p, e, "*** signal=");
    p = addnum(p, e, sig);
    p = addstr(p, e, " pid=");
    p = addnum(p, e, getpid());
    p = addstr(p, e, "\n");
    if (write(STDERR_FILENO, m, p - m) < 0) {
        // dontcare
    }
#endif

    char v = (char)sig;
    if (write(Daemon::instance()->sigPipeFd(), &v, 1) != 1) {
        /* If we can't write to internal signal forwarding
         * pipe, we might as well quit */
        const char m[] = "*** signal pipe write failure - terminating\n";
        if (write(STDERR_FILENO, m, sizeof m - 1) == -1) {
            // dontcare
        }
        _exit(EXIT_FAILURE);
    }
}

static int read_from_signal_pipe(int fd)
{
    char sig = 0;
    if (read(fd, &sig, 1) != 1) {
        /* If we can't read from internal signal forwarding
         * pipe, we might as well quit */
        Logger::logError("signal pipe read failure - terminating\n");
        exit(EXIT_FAILURE);
    }
    return sig;
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

    debug("trying to disconnect booster socket...\n");

    unsigned started = timestamp();
    unsigned timeout = 5000;

    if (shutdown(socket_fd, SHUT_WR) == -1) {
        warning("socket shutdown failed: %m\n");
        goto EXIT;
    }

    for (;;) {
        unsigned elapsed = timestamp() - started;
        if (elapsed >= timeout)
            break;

        struct pollfd pfd = {
            .fd = socket_fd,
            .events = POLLIN,
            .revents = 0,
        };

        debug("waiting for booster socket input...\n");
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
        debug("booster socket was succesfully disconnected\n");
    else
        warning("could not disconnect booster socket\n");

    return disconnected;
}

static void kill_process(const char *label, pid_t pid)
{
    if (pid == -1) {
        warning("%s pid is not known, can't kill it", label);
        goto EXIT;
    }

    warning("sending SIGTERM to %s (pid=%d)", label, (int)pid);

    if (kill(pid, SIGTERM) == -1)
        goto FAIL;

    for (int i = 0; i < 10; ++i) {
        sleep(1);
        /* We use this function for both boosters and invokers.
         * Boosters are child processes and we must use wait*()
         * to reap them. But invokers are not descendants of
         * booster daemon, so ECHILD failures are expected too.
         */
        int status = 0;
        if (waitpid(pid, &status, WNOHANG) == -1 && errno != ECHILD)
            warning("%s pid=%d) waitpid error: %m", label, (int)pid);
        if (kill(pid, 0) == -1)
            goto FAIL;
    }

    warning("sending SIGKILL to %s (pid=%d)", label, (int)pid);

    if (kill(pid, SIGKILL) == -1)
        goto FAIL;

    for (int i = 0; i < 10; ++i) {
        sleep(1);
        int status = 0;
        if (waitpid(pid, &status, WNOHANG) == -1 && errno != ECHILD)
            warning("%s pid=%d) waitpid error: %m", label, (int)pid);
        if (kill(pid, 0) == -1)
            goto FAIL;
    }

    warning("%s (pid=%d) did not exit", label, (int)pid);
    goto EXIT;

FAIL:
    if (errno == ESRCH)
        debug("%s (pid=%d) has exited", label, (int)pid);
    else
        warning("%s (pid=%d) kill failed: %m", label, (int)pid);

EXIT:
    return;
}

static void close_invoker(pid_t invoker_pid, int socket_fd, int exit_status)
{
    if (socket_fd != -1) {
        Logger::logWarning("Daemon: sending exit(%d) to invoker(%d)\n",
                           exit_status, (int)invoker_pid);
        uint32_t msg = INVOKER_MSG_EXIT;
        uint32_t dta = exit_status;
        write_dontcare(socket_fd, &msg, sizeof msg);
        write_dontcare(socket_fd, &dta, sizeof dta);

        if (shutdown_socket(socket_fd))
            invoker_pid = -1;

        close(socket_fd),
            socket_fd = -1;
    }

    if (invoker_pid != -1)
        kill_process("invoker", invoker_pid);
}

Daemon::Daemon(int & argc, char * argv[]) :
    m_daemon(false),
    m_debugMode(false),
    m_bootMode(false),
    m_boosterPid(0),
    m_socketManager(new SocketManager),
    m_singleInstance(new SingleInstance),
    m_notifySystemd(false),
    m_booster(0)
{
    // Open the log
    Logger::openLog(argc > 0 ? argv[0] : "booster");
    Logger::logDebug("starting..");

    // Install signal handlers. The original handlers are saved
    // in the daemon instance so that they can be restored in boosters.
    setUnixSignalHandler(SIGCHLD, write_to_signal_pipe); // reap zombies
    setUnixSignalHandler(SIGINT,  write_to_signal_pipe); // exit launcher
    setUnixSignalHandler(SIGTERM, write_to_signal_pipe); // exit launcher
    setUnixSignalHandler(SIGUSR1, write_to_signal_pipe); // enter normal mode from boot mode
    setUnixSignalHandler(SIGUSR2, write_to_signal_pipe); // enter boot mode (same as --boot-mode)
    setUnixSignalHandler(SIGPIPE, write_to_signal_pipe); // broken invoker's pipe
    setUnixSignalHandler(SIGHUP,  write_to_signal_pipe); // re-exec

    if (!Daemon::m_instance)
    {
        Daemon::m_instance = this;
    }
    else
    {
        throw std::runtime_error("Daemon: Daemon already created!\n");
    }

    // Store arguments list
    m_initialArgv = argv;
    m_initialArgc = argc;

    // Parse arguments
    parseArgs(argc, argv);

    if (socketpair(AF_UNIX, SOCK_DGRAM, 0, m_boosterLauncherSocket) == -1)
    {
        throw std::runtime_error("Daemon: Creating a socket pair for boosters failed!\n");
    }

    if (pipe(m_sigPipeFd) == -1)
    {
        throw std::runtime_error("Daemon: Creating a pipe for Unix signals failed!\n");
    }
}

Daemon * Daemon::instance()
{
    return Daemon::m_instance;
}

void Daemon::run(Booster *booster)
{
    m_booster = booster;

    if (!m_boostedApplication.empty())
        m_booster->setBoostedApplication(m_boostedApplication);

    // Make sure that LD_BIND_NOW does not prevent dynamic linker to
    // use lazy binding in later dlopen() calls.
    unsetenv("LD_BIND_NOW");

    // dlopen single-instance
    loadSingleInstancePlugin();

    // Create socket for the booster
    Logger::logDebug("Daemon: initing socket: %s", booster->boosterType().c_str());
    m_socketManager->initSocket(booster->socketId());

    // Daemonize if desired
    if (m_daemon)
    {
        daemonize();
    }

    // Fork each booster for the first time
    Logger::logDebug("Daemon: forking booster: %s", booster->boosterType().c_str());
    forkBooster();

    // Notify systemd that init is done
    if (m_notifySystemd) {
        Logger::logDebug("Daemon: initialization done. Notify systemd\n");
        sd_notify(0, "READY=1");
    }

    // Main loop
    while (true)
    {
        // Variables used by the select call
        fd_set rfds;
        int ndfs = 0;

        // Init data for select
        FD_ZERO(&rfds);

        FD_SET(m_boosterLauncherSocket[0], &rfds);
        ndfs = std::max(ndfs, m_boosterLauncherSocket[0]);

        FD_SET(m_sigPipeFd[0], &rfds);
        ndfs = std::max(ndfs, m_sigPipeFd[0]);

        /* Listen to invoker EOFs */
        for (auto iter = m_boosterPidToInvokerFd.begin(); iter != m_boosterPidToInvokerFd.end(); ++iter) {
            int fd = iter->second;
            if (fd != -1) {
                FD_SET(fd, &rfds);
                ndfs = std::max(ndfs, fd);
            }
        }

        // Wait for something appearing in the pipes.
        if (select(ndfs + 1, &rfds, NULL, NULL, NULL) > 0)
        {
            Logger::logDebug("Daemon: select done.");

            // Check if a booster died
            if (FD_ISSET(m_boosterLauncherSocket[0], &rfds))
            {
                Logger::logDebug("Daemon: FD_ISSET(m_boosterLauncherSocket[0])");
                readFromBoosterSocket(m_boosterLauncherSocket[0]);
            }

            // Check if we got SIGCHLD, SIGTERM, SIGUSR1 or SIGUSR2
            if (FD_ISSET(m_sigPipeFd[0], &rfds))
            {
                Logger::logDebug("Daemon: FD_ISSET(m_sigPipeFd[0])");
                int dataReceived = read_from_signal_pipe(m_sigPipeFd[0]);

                switch (dataReceived)
                {
                case SIGCHLD:
                    Logger::logDebug("Daemon: SIGCHLD received.");
                    reapZombies();
                    break;

                case SIGINT:
                case SIGTERM: {
                    Logger::logDebug("Daemon: SIGINT / SIGTERM received.");

                    // FIXME: Legacy pid file path -> see daemonize()
                    const std::string pidFilePath = m_socketManager->socketRootPath() + m_booster->boosterType() + ".pid";
                    FILE * const pidFile = fopen(pidFilePath.c_str(), "r");
                    if (pidFile)
                    {
                        pid_t filePid;
                        if (fscanf(pidFile, "%d\n", &filePid) == 1 && filePid == getpid())
                        {
                            unlink(pidFilePath.c_str());
                        }
                        fclose(pidFile);
                    }

                    for (;;) {
                        PidVect::iterator iter(m_children.begin());
                        if (iter == m_children.end())
                            break;
                        pid_t booster_pid = *iter;
                        m_children.erase(iter);

                        /* Get and remove booster socket  fd */
                        int socket_fd = -1;
                        FdMap::iterator fdIter = m_boosterPidToInvokerFd.find(booster_pid);
                        if (fdIter != m_boosterPidToInvokerFd.end()) {
                            socket_fd = (*fdIter).second;
                            m_boosterPidToInvokerFd.erase(fdIter);
                        }
                        /* Get and remove invoker pid */
                        pid_t invoker_pid = -1;
                        PidMap::iterator pidIter = m_boosterPidToInvokerPid.find(booster_pid);
                        if (pidIter != m_boosterPidToInvokerPid.end()) {
                            invoker_pid = (*pidIter).second;
                            m_boosterPidToInvokerPid.erase(pidIter);
                        }

                        /* Normally boosters are stopped on shutdown / user switch,
                         * and even then it should happen after applications have
                         * already been stopped.
                         */
                        warning("terminating: booster:%d invoker:%d socket:%d",
                                (int)booster_pid, (int)invoker_pid, socket_fd);

                        /* Terminate invoker */
                        close_invoker(invoker_pid, socket_fd, EXIT_FAILURE);

                        /* Terminate booster */
                        kill_process("booster", booster_pid);
                    }

                    Logger::logDebug("booster exit");
                    exit(EXIT_SUCCESS);
                    break;
                }

                case SIGUSR1:
                    Logger::logDebug("Daemon: SIGUSR1 received.");
                    enterNormalMode();
                    break;

                case SIGUSR2:
                    Logger::logDebug("Daemon: SIGUSR2 received.");
                    enterBootMode();
                    break;

                case SIGPIPE:
                    Logger::logDebug("Daemon: SIGPIPE received.");
                    break;

                default:
                    break;
                }
                continue;
            }

            /* Handle closed booster sockets */
            for (auto iter = m_boosterPidToInvokerFd.begin(); iter != m_boosterPidToInvokerFd.end();) {
                int socket_fd = iter->second;
                if (socket_fd != -1 && FD_ISSET(socket_fd, &rfds)) {
                    /* Note: bookkeeping must be updated first to avoid
                     *       any ringing due to socket closes / child
                     *       process exits.
                     */
                    pid_t booster_pid = iter->first;
                    iter = m_boosterPidToInvokerFd.erase(iter);

                    pid_t invoker_pid = -1;
                    PidMap::iterator pidIter = m_boosterPidToInvokerPid.find(booster_pid);
                    if (pidIter != m_boosterPidToInvokerPid.end()) {
                        invoker_pid = pidIter->second;
                        m_boosterPidToInvokerPid.erase(pidIter);
                    }

                    /* Note that it is slightly unexpected if we get here
                     * as it means invoker exited rather than application.
                     */
                    warning("terminating: booster:%d invoker:%d socket:%d",
                            (int)booster_pid, (int)invoker_pid, socket_fd);

                    /* Terminate invoker */
                    close_invoker(invoker_pid, socket_fd, EXIT_FAILURE);

                    /* Terminate booster */
                    kill_process("booster", booster_pid);
                } else {
                    ++iter;
                }
            }
        }
    }
}

void Daemon::readFromBoosterSocket(int fd)
{
    pid_t invokerPid = 0;
    int delay = 0;
    int socketFd = -1;

    struct iovec iov[2];
    char buf[CMSG_SPACE(sizeof socketFd)];
    struct msghdr msg;
    struct cmsghdr *cmsg;

    memset(iov, 0, sizeof iov);
    memset(buf, 0, sizeof buf);
    memset(&msg, 0, sizeof msg);

    iov[0].iov_base = &invokerPid;
    iov[0].iov_len = sizeof invokerPid;
    iov[1].iov_base = &delay;
    iov[1].iov_len = sizeof delay;

    msg.msg_iov        = iov;
    msg.msg_iovlen     = 2;
    msg.msg_name       = NULL;
    msg.msg_namelen    = 0;
    msg.msg_control    = buf;
    msg.msg_controllen = sizeof buf;

    if (recvmsg(fd, &msg, 0) == -1) {
        Logger::logError("Daemon: Critical error communicating with booster. Exiting applauncherd.\n");
        exit(EXIT_FAILURE);
    }

    for (cmsg = CMSG_FIRSTHDR(&msg); cmsg; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
        if (cmsg->cmsg_level == SOL_SOCKET &&
            cmsg->cmsg_type == SCM_RIGHTS &&
            cmsg->cmsg_len >= CMSG_LEN(sizeof(socketFd))) {
            memcpy(&socketFd, CMSG_DATA(cmsg), sizeof socketFd);
        }
    }

    Logger::logDebug("Daemon: booster=%d invoker=%d socket=%d delay=%d\n",
                     m_boosterPid, invokerPid, socketFd, delay);

    if (m_boosterPid > 0) {
        /* We were expecting booster details => update bookkeeping */
        if (socketFd != -1) {
            // Store booster pid - invoker socket pair
            m_boosterPidToInvokerFd[m_boosterPid] = socketFd, socketFd = -1;
        }
        if (invokerPid > 0) {
            // Store booster pid - invoker pid pair
            m_boosterPidToInvokerPid[m_boosterPid] = invokerPid;
        }
    }

    if (socketFd != -1) {
        /* If we are not going to use the received fd, it needs to be closed */
        Logger::logWarning("Daemon: close stray socket file descriptor: %d\n", socketFd);
        close(socketFd);
    }

    // 2nd param guarantees some time for the just launched application
    // to start up before forking new booster. Not doing this would
    // slow down the start-up significantly on single core CPUs.

    forkBooster(delay);
}

void Daemon::killProcess(pid_t pid, int signal) const
{
    if (pid > 0)
    {
        Logger::logWarning("Daemon: Killing pid %d with %d", pid, signal);
        if (kill(pid, signal) != 0)
        {
            Logger::logError("Daemon: Failed to kill %d: %s\n",
                             pid, strerror(errno));
        }
    }
}

void Daemon::loadSingleInstancePlugin()
{
    void * handle = dlopen(SINGLE_INSTANCE_PATH, RTLD_NOW);
    if (!handle)
    {
        Logger::logWarning("Daemon: dlopening single-instance failed: %s", dlerror());
    }
    else
    {
        if (m_singleInstance->validateAndRegisterPlugin(handle))
        {
            Logger::logDebug("Daemon: single-instance plugin loaded.'");
        }
        else
        {
            Logger::logWarning("Daemon: Invalid single-instance plugin: '%s'",
                               SINGLE_INSTANCE_PATH);
        }
    }
}

void Daemon::dropCapabilities()
{
    cap_t caps = cap_init();

    if (!caps || cap_set_proc(caps) == -1) {
        Logger::logError("Daemon: Failed to drop capabilities");
    }

    if (caps) {
        cap_free(caps);
    }
}

void Daemon::forkBooster(int sleepTime)
{
    if (!m_booster) {
        // Critical error unknown booster type. Exiting applauncherd.
        _exit(EXIT_FAILURE);
    }

    // Invalidate current booster pid
    m_boosterPid = 0;

    // Fork a new process
    pid_t newPid = fork();

    if (newPid == -1)
        throw std::runtime_error("Daemon: Forking while invoking");

    if (newPid == 0) /* Child process */
    {
        // Will be reopened with new identity when/if
        // there is something to report
        Logger::closeLog();

        // Restore used signal handlers
        restoreUnixSignalHandlers();

        // Will get this signal if applauncherd dies
        prctl(PR_SET_PDEATHSIG, SIGHUP);

        // Close unused read end of the booster socket
        close(m_boosterLauncherSocket[0]);

        // Close signal pipe
        close(m_sigPipeFd[0]);
        close(m_sigPipeFd[1]);

        // Close socket file descriptors
        FdMap::iterator i(m_boosterPidToInvokerFd.begin());
        while (i != m_boosterPidToInvokerFd.end())
        {
            if ((*i).second != -1) {
                close((*i).second);
                (*i).second = -1;
            }
            i++;
        }
        // Set session id
        if (setsid() < 0)
            Logger::logError("Daemon: Couldn't set session id\n");

        // Guarantee some time for the just launched application to
        // start up before initializing new booster if needed.
        // Not done if in the boot mode.
        if (!m_bootMode && sleepTime) {
            Logger::logDebug("allow time for application startup - sleep(%ds)...\n", sleepTime);
            sleep(sleepTime);
        }

        Logger::logDebug("Daemon: Running a new Booster of type '%s'", m_booster->boosterType().c_str());

        // Initialize and wait for commands from invoker
        try {
            m_booster->initialize(m_initialArgc, m_initialArgv, m_boosterLauncherSocket[1],
                                  m_socketManager->findSocket(m_booster->socketId()),
                                  m_singleInstance, m_bootMode);
        } catch (const std::runtime_error &e) {
            Logger::logError("Booster: Failed to initialize: %s\n", e.what());
            delete m_booster;
            _exit(EXIT_FAILURE);
        }

        m_instance = NULL;

        // No need for capabilities anymore
        dropCapabilities();

        // Run the current Booster
        int retval = m_booster->run(m_socketManager);

        // Finish
        delete m_booster;

        // _exit() instead of exit() to avoid situation when destructors
        // for static objects may be run incorrectly
        _exit(retval);
    }
    else /* Parent process */
    {
        // Store the pid so that we can reap it later
        m_children.push_back(newPid);

        // Set current process ID globally to the given booster type
        // so that we now which booster to restart when booster exits.
        m_boosterPid = newPid;
    }
}

void Daemon::reapZombies()
{
    // Loop through all child pid's and wait for them with WNOHANG.
    PidVect::iterator i(m_children.begin());
    while (i != m_children.end())
    {
        // Check if the pid had exited and become a zombie
        int status = 0;
        pid_t pid = waitpid(*i, &status, WNOHANG);
        if (pid > 0)
        {
            // The pid had exited. Remove it from the pid vector.
            i = m_children.erase(i);

            // Find out what happened
            int exit_status = EXIT_FAILURE;
            int signal_no = 0;

            if (WIFSIGNALED(status)) {
                signal_no = WTERMSIG(status);
                Logger::logWarning("boosted process (pid=%d) signal(%s)\n",
                                   pid, strsignal(signal_no));
            } else if (WIFEXITED(status)) {
                exit_status = WEXITSTATUS(status);
                if (exit_status != EXIT_SUCCESS)
                    Logger::logWarning("Boosted process (pid=%d) exit(%d)\n",
                                       pid, exit_status);
                else
                    Logger::logDebug("Boosted process (pid=%d) exit(%d)\n",
                                     pid, exit_status);
            }

            /* Get and remove booster socket fd */
            int socket_fd = -1;
            FdMap::iterator fdIter = m_boosterPidToInvokerFd.find(pid);
            if (fdIter != m_boosterPidToInvokerFd.end()) {
                socket_fd = (*fdIter).second;
                m_boosterPidToInvokerFd.erase(fdIter);
            }

            /* Get and remove invoker pid */
            pid_t invoker_pid = -1;
            PidMap::iterator pidIter = m_boosterPidToInvokerPid.find(pid);
            if (pidIter != m_boosterPidToInvokerPid.end()) {
                invoker_pid = (*pidIter).second;
                m_boosterPidToInvokerPid.erase(pidIter);
            }

            /* Terminate invoker associated with the booster */
            close_invoker(invoker_pid, socket_fd, exit_status);

            // Check if pid belongs to a booster and restart the dead booster if needed
            if (pid == m_boosterPid)
            {
                forkBooster(m_boosterSleepTime);
            }
        }
        else
        {
            i++;
        }
    }

    /* Handle stray child exits */
    for (;;) {
        int status = 0;
        pid_t pid = waitpid(-1, &status, WNOHANG);
        if (pid <= 0)
            break;
        Logger::logWarning("unexpected child exit pid=%d status=0x%x\n", pid, status);
    }
}

void Daemon::daemonize()
{
    // Our process ID and Session ID
    pid_t pid, sid;

    // Fork off the parent process: first fork
    pid = fork();
    if (pid < 0)
        throw std::runtime_error("Daemon: Unable to fork daemon");

    // If we got a good PID, then we can exit the parent process.
    if (pid > 0)
    {
        // Wait for the child fork to exit to ensure the PID has been written before a caller
        // is notified of the exit.
        waitpid(pid, NULL, 0);
        _exit(EXIT_SUCCESS);
    }

    // Fork off the parent process: second fork
    pid = fork();
    if (pid < 0)
        throw std::runtime_error("Daemon: Unable to fork daemon");

    // If we got a good PID, then we can exit the parent process.
    if (pid > 0)
    {
        /* FIXME: Existing booster systemd unit files etc are expecting
         *        pid file paths derived from booster type. As long as
         *        there are no application specific boosters using
         *        pid files we can continue using old style paths.
         */
#if 0
        // Path that takes also application name into account
        const std::string pidFilePath = m_socketManager->socketRootPath() + m_booster->socketId() + ".pid";
#else
        // Legacy path
        const std::string pidFilePath = m_socketManager->socketRootPath() + m_booster->boosterType() + ".pid";
#endif
        FILE * const pidFile = fopen(pidFilePath.c_str(), "w");
        if (pidFile)
        {
            fprintf(pidFile, "%d\n", pid);
            fclose(pidFile);
        }

        _exit(EXIT_SUCCESS);
    }

    // Change the file mode mask
    umask(0);

    // Open any logs here

    // Create a new SID for the child process
    sid = setsid();
    if (sid < 0)
        throw std::runtime_error("Daemon: Unable to setsid.");

    // Change the current working directory
    if ((chdir("/")) < 0)
        throw std::runtime_error("Daemon: Unable to chdir to '/'");

    // Open file descriptors pointing to /dev/null
    // Redirect standard file descriptors to /dev/null
    // Close new file descriptors

    const int new_stdin = open("/dev/null", O_RDONLY);
    if (new_stdin != -1) {
        dup2(new_stdin, STDIN_FILENO);
        close(new_stdin);
    }

    const int new_stdout = open("/dev/null", O_WRONLY);
    if (new_stdout != -1) {
        dup2(new_stdout, STDOUT_FILENO);
        close(new_stdout);
    }

    const int new_stderr = open("/dev/null", O_WRONLY);
    if (new_stderr != -1) {
        dup2(new_stderr, STDERR_FILENO);
        close(new_stderr);
    }
}


void Daemon::parseArgs(int argc, char **argv)
{
    // Options recognized
    static const struct option longopts[] = {
        { "help",             no_argument,       NULL, 'h' },
        { "verbose",          no_argument,       NULL, 'v' },
        { "debug",            no_argument,       NULL, 'v' },
        { "boot-mode",        no_argument,       NULL, 'b' },
        { "daemon",           no_argument,       NULL, 'd' },
        { "systemd",          no_argument,       NULL, 'n' },
        { "application",      required_argument, NULL, 'a' },
        { 0, 0, 0, 0}
    };
    static const char shortopts[] =
        "+"  // use posix rules
        "h"  // --help
        "v"  // --verbose --debug
        "b"  // --boot-mode
        "d"  // --daemon
        "n"  // --systemd
        "a:" // --application=<APP>
        ;
    for (;;) {
        int opt = getopt_long(argc, argv, shortopts, longopts, NULL);
        if (opt == -1)
            break;
        switch (opt) {
        case 'h':
            usage(*argv, EXIT_SUCCESS);
            break;
        case 'v':
            Logger::setDebugMode(true);
            m_debugMode = true;
            break;
        case 'b':
            Logger::logInfo("Daemon: Boot mode set.");
            m_bootMode = true;
            break;
        case 'd':
            m_daemon = true;
            break;
        case 'n':
            m_notifySystemd = true;
            break;
        case 'a':
            m_boostedApplication = optarg;
            break;
        default:
        case '?':
            usage(*argv, EXIT_FAILURE);
        }
    }
    if (optind < argc)
        usage(*argv, EXIT_FAILURE);
}

// Prints the usage and exits with given status
void Daemon::usage(const char *name, int status)
{
    char *nameCopy = strdup(name);
    name = basename(nameCopy);
    printf("\n"
           "Start the application launcher daemon.\n"
           "\n"
           "Usage:\n"
           "  %s [options]\n"
           "\n"
           "Options:\n"
           "  -b, --boot-mode\n"
           "                   Start %s in the boot mode. This means that\n"
           "                   boosters will not initialize caches and booster\n"
           "                   respawn delay is set to zero.\n"
           "                   Normal mode is restored by sending SIGUSR1\n"
           "                   to the launcher.\n"
           "                   Boot mode can be activated also by sending SIGUSR2\n"
           "                   to the launcher.\n"
           "  -d, --daemon\n"
           "                   Run as %s a daemon.\n"
           "  -a, --application=<application>\n"
           "                   Run as application specific booster.\n"
           "  -n, --systemd\n"
           "                   Notify systemd when initialization is done\n"
           "  -h, --help\n"
           "                   Print this help.\n"
           "  -v, --verbose, --debug\n"
           "                   Make diagnostic logging more verbose.\n"
           "\n",
           name, name, name);

    free(nameCopy);

    exit(status);
}

int Daemon::sigPipeFd() const
{
    return m_sigPipeFd[1];
}

void Daemon::enterNormalMode()
{
    if (m_bootMode)
    {
        m_bootMode = false;

        // Kill current boosters
        killBoosters();

        Logger::logInfo("Daemon: Exited boot mode.");
    }
    else
    {
        Logger::logInfo("Daemon: Already in normal mode.");
    }
}

void Daemon::enterBootMode()
{
    if (!m_bootMode)
    {
        m_bootMode = true;

        // Kill current boosters
        killBoosters();

        Logger::logInfo("Daemon: Entered boot mode.");
    }
    else
    {
        Logger::logInfo("Daemon: Already in boot mode.");
    }
}

void Daemon::killBoosters()
{
    if (m_boosterPid > 0)
        killProcess(m_boosterPid, SIGTERM);

    // NOTE!!: m_boosterPid must not be cleared
    // in order to automatically start new boosters.
}

void Daemon::setUnixSignalHandler(int signum, sighandler_t handler)
{
    // needs to be undone in instance
    m_originalSigHandlers[signum] = SIG_DFL;

    struct sigaction sig;
    memset(&sig, 0, sizeof(sig));
    sig.sa_flags = SA_RESTART;
    sig.sa_handler = handler;
    if (sigaction(signum, &sig, NULL) == -1)
        warning("trap(%s): %m", strsignal(signum));
    else
        debug("trap(%s): ok", strsignal(signum));
}

void Daemon::restoreUnixSignalHandlers()
{
    for (SigHandlerMap::iterator it = m_originalSigHandlers.begin(); it != m_originalSigHandlers.end(); it++ )
    {
        int signum = it->first;
        if (signal(signum, SIG_DFL) == SIG_ERR)
            warning("untrap(%s): %m", strsignal(signum));
        else
            debug("untrap(%s): ok", strsignal(signum));
    }
    m_originalSigHandlers.clear();
}


Daemon::~Daemon()
{
    delete m_socketManager;
    delete m_singleInstance;

    Logger::closeLog();
}
