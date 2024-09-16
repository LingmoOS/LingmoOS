/* SPDX-FileCopyrightText: None */
/* SPDX-License-Identifier: MIT */
#ifndef KILL_H
#define KILL_H

#include "meminfo.h"

#include <regex.h>
#include <stdbool.h>

typedef struct
{
    /* if the available memory AND swap goes below these percentages,
     * we start killing processes */
    double mem_term_percent;
    double mem_kill_percent;
    double swap_term_percent;
    double swap_kill_percent;
    /* send d-bus notifications? */
    char *notify_ext;
    /* kill all processes within a process group */
    bool kill_process_group;
    /* do not kill processes owned by root */
    bool ignore_root_user;
    /* prefer/avoid killing these processes. NULL = no-op. */
    regex_t *prefer_regex;
    regex_t *avoid_regex;
    /* will ignore these processes. NULL = no-op. */
    regex_t *ignore_regex;
    /* memory report interval, in milliseconds */
    double report_interval_ms;
    /* Flag --dryrun was passed */
    bool dryrun;
} poll_loop_args_t;

void kill_process(const poll_loop_args_t *args, int sig, const procinfo_t *victim);
procinfo_t find_largest_process(const poll_loop_args_t *args);

#endif
