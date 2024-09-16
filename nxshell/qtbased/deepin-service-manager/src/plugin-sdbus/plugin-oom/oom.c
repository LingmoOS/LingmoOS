/* SPDX-FileCopyrightText: None */
/* SPDX-License-Identifier: MIT */

#include "oom.h"

#include "3rdparty/earlyoom/globals.h"
#include "3rdparty/earlyoom/kill.h"
#include "3rdparty/earlyoom/msg.h"
#include "dconfig.h"

#include <sys/prctl.h>

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <time.h>
#include <unistd.h>

static int set_oom_score_adj(int);
static void poll_loop(const poll_loop_args_t *args);

void start_oom(poll_loop_args_t *args)
{
    signal(SIGCHLD, SIG_IGN);
    if (chdir(procdir_path) != 0) {
        fatal(4, "Could not cd to /proc: %s", strerror(errno));
        return;
    }
    prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_CLEAR_ALL, 0, 0, 0);
    meminfo_t m = parse_meminfo();
    { // sysctl
        bool fail = false;
        if (setpriority(PRIO_PROCESS, 0, -20) != 0) {
            warn("Could not set priority: %s. Continuing anyway\n", strerror(errno));
            fail = true;
        }
        int ret = set_oom_score_adj(-100);
        if (ret != 0) {
            warn("Could not set oom_score_adj: %s. Continuing anyway\n", strerror(ret));
            fail = true;
        }
        if (!fail) {
            fprintf(stderr, "Priority was raised successfully\n");
        }
    }
    // Print memory limits
    fprintf(stderr,
            "mem total: %4lld MiB, user mem total: %4lld MiB, swap total: %4lld MiB\n",
            m.MemTotalKiB / 1024,
            m.UserMemTotalKiB / 1024,
            m.SwapTotalKiB / 1024);
    fprintf(stderr,
            "sending SIGTERM when mem <= %f%% and swap <= %f%%\n",
            args->mem_term_percent,
            args->swap_term_percent);
    fprintf(stderr,
            "        SIGKILL when mem <= %f%% and swap <= %f%%\n",
            args->mem_kill_percent,
            args->swap_kill_percent);
    int err = mlockall(MCL_CURRENT | MCL_FUTURE | MCL_ONFAULT);
    if (err != 0) {
        perror("Could not lock memory - continuing anyway");
    }

    // Jump into main poll loop
    poll_loop(args);
}

// Returns errno (success = 0)
static int set_oom_score_adj(int oom_score_adj)
{
    char buf[PATH_LEN] = { 0 };
    pid_t pid = getpid();

    snprintf(buf, sizeof(buf), "%s/%d/oom_score_adj", procdir_path, pid);
    FILE *f = fopen(buf, "w");
    if (f == NULL) {
        return -1;
    }

    // fprintf returns a negative error code on failure
    int ret1 = fprintf(f, "%d", oom_score_adj);
    // fclose returns a non-zero value on failure and errno contains the error code
    int ret2 = fclose(f);

    if (ret1 < 0) {
        return -ret1;
    }
    if (ret2) {
        return errno;
    }
    return 0;
}

/* Calculate the time we should sleep based upon how far away from the memory and swap
 * limits we are (headroom). Returns a millisecond value between 100 and 1000 (inclusive).
 * The idea is simple: if memory and swap can only fill up so fast, we know how long we can sleep
 * without risking to miss a low memory event.
 */
static unsigned sleep_time_ms(const poll_loop_args_t *args, const meminfo_t *m)
{
    // Maximum expected memory/swap fill rate. In kiB per millisecond ==~ MiB per second.
    const long long mem_fill_rate = 6000; // 6000MiB/s seen with "stress -m 4 --vm-bytes 4G"
    const long long swap_fill_rate = 800; //  800MiB/s seen with membomb on ZRAM
    // Clamp calculated value to this range (milliseconds)
    const unsigned min_sleep = 100;
    const unsigned max_sleep = 1000;

    long long mem_headroom_kib = (long long)((m->MemAvailablePercent - args->mem_term_percent)
                                             * (double)m->UserMemTotalKiB / 100);
    if (mem_headroom_kib < 0) {
        mem_headroom_kib = 0;
    }
    long long swap_headroom_kib =
        (long long)((m->SwapFreePercent - args->swap_term_percent) * (double)m->SwapTotalKiB / 100);
    if (swap_headroom_kib < 0) {
        swap_headroom_kib = 0;
    }
    long long ms = mem_headroom_kib / mem_fill_rate + swap_headroom_kib / swap_fill_rate;
    if (ms < min_sleep) {
        return min_sleep;
    }
    if (ms > max_sleep) {
        return max_sleep;
    }
    return (unsigned)ms;
}

/* lowmem_sig compares the limits with the current memory situation
 * and returns which signal (SIGKILL, SIGTERM, 0) should be sent in
 * response. 0 means that there is enough memory and we should
 * not kill anything.
 */
static int lowmem_sig(const poll_loop_args_t *args, const meminfo_t *m)
{
    if (m->MemAvailablePercent <= args->mem_kill_percent
        && m->SwapFreePercent <= args->swap_kill_percent)
        return SIGKILL;
    else if (m->MemAvailablePercent <= args->mem_term_percent
             && m->SwapFreePercent <= args->swap_term_percent)
        return SIGTERM;
    return 0;
}

// poll_loop is the main event loop. Never returns.
static void poll_loop(const poll_loop_args_t *args)
{
    // Print a a memory report when this reaches zero. We start at zero so
    // we print the first report immediately.
    int report_countdown_ms = 0;

    while (1) {
        meminfo_t m = parse_meminfo();
        int sig = lowmem_sig(args, &m);
        if (sig == SIGKILL) {
            print_mem_stats(warn, m);
            warn("low memory! at or below SIGKILL limits: mem %f%%, swap %f%%\n",
                 args->mem_kill_percent,
                 args->swap_kill_percent);
        } else if (sig == SIGTERM) {
            print_mem_stats(warn, m);
            warn("low memory! at or below SIGTERM limits: mem %f%%, swap %f%%\n",
                 args->mem_term_percent,
                 args->swap_term_percent);
        }
        if (sig) {
            procinfo_t victim = find_largest_process(args);
            /* The run time of find_largest_process is proportional to the number
             * of processes, and takes 2.5ms on my box with a running Gnome desktop (try "make
             * bench"). This is long enough that the situation may have changed in the meantime, so
             * we double-check if we still need to kill anything. The run time of parse_meminfo is
             * only 6us on my box and independent of the number of processes (try "make bench").
             */
            m = parse_meminfo();
            if (lowmem_sig(args, &m) == 0) {
                warn("memory situation has recovered while selecting victim\n");
            } else {
                kill_process(args, sig, &victim);
            }
        } else if (args->report_interval_ms && report_countdown_ms <= 0) {
            report_countdown_ms = args->report_interval_ms;
        }
        unsigned sleep_ms = sleep_time_ms(args, &m);
        struct timespec req = { .tv_sec = (time_t)(sleep_ms / 1000),
                                .tv_nsec = (sleep_ms % 1000) * 1000000 };
        while (nanosleep(&req, &req) == -1 && errno == EINTR)
            ;
        report_countdown_ms -= (int)sleep_ms;
    }
}
