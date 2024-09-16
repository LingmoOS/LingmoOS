// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYNCSTATUS_H
#define SYNCSTATUS_H

#include <memory>
#include <functional>
#include <string>

#define BLOCK_SIZE 4096

static const std::string s_headerInfos[] = {"webstart", "webfinish", "webindex"};

enum INFOHEAD {
    INFO_WEB_START = 0,
    INFO_WEB_FINISH = 1,
    INFO_WEB_INDEX = 2,
    INFO_WEB_MAX = 3,
};

enum HandleResult {
    RES_OKHEADER = 200,
    RES_NOTFOUND = 404,
    RES_ERROR = 444,
    RES_BODY = 555,
    RES_FINISH = 666,
    RES_INDEX_CHANGE = 668,
    RES_WEB_START = 669,
    RES_WEB_FINISH = 670,
};

using ResponseHandler = std::function<bool(int status, const char *buffer, size_t size)>;

enum WebState {
    WEB_DISCONNECTED = -2,
    WEB_IO_ERROR = -1,
    WEB_NOT_FOUND = 0,
    WEB_CONNECTED = 1,
    WEB_TRANS_START = 2,  // 整个传输开始
    WEB_TRANS_FINISH = 3, // 整个传输完成
    WEB_INDEX_BEGIN = 4,   // 某一选择项（文件或目录）传输开始
    WEB_INDEX_END = 5,     // 某一选择项（文件或目录）传输结束
    WEB_FILE_BEGIN = 6,   // 某一文件传输开始
    WEB_FILE_END = 7,     // 某一文件传输结束
};

struct sync_stats_s {
    int64_t all_total_size;   // 总量
    int64_t all_current_size;   // 当前已接收量
    int64_t cast_time_ms;   // 最大已用时间
};

class ProgressCallInterface : public std::enable_shared_from_this<ProgressCallInterface>
{
public:
    virtual bool onProgress(uint64_t size) = 0;

    virtual void onWebChanged(int state, std::string msg = "", uint64_t size = 0) = 0;
};


class WebInterface
{
public:
    void setCallback(std::shared_ptr<ProgressCallInterface> callback)
    {
        _callback = callback;
    }

protected:
    std::shared_ptr<ProgressCallInterface> _callback { nullptr };
};


#endif // SYNCSTATUS_H
