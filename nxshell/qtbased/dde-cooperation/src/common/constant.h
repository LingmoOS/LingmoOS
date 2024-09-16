// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONSTANT_H
#define CONSTANT_H

#define UNI_CHANNEL  "_cooperation._udp"
#define UNI_RPC_PORT_UDP  51595

#define UNI_SHARE_SERVER_PORT 24802

typedef enum trans_result_t {
    TRANS_CANCELED = 48,
    TRANS_EXCEPTION = 49,
    TRANS_COUNT_SIZE = 50,
    TRANS_WHOLE_START = 51,
    TRANS_WHOLE_FINISH = 52,
    TRANS_INDEX_CHANGE = 53,
    TRANS_FILE_CHANGE = 54,
    TRANS_FILE_SPEED = 55,
    TRANS_FILE_DONE = 56,
} TransResult;

// compat old protocol transfer status
typedef enum job_status_t {
    JOB_TRANS_FAILED = -1,
    JOB_TRANS_DOING = 11,
    JOB_TRANS_FINISHED = 12,
    JOB_TRANS_CANCELED = 13,
} JobStatus;

#endif // CONSTANT_H
