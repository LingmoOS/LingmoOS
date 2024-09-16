// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EVENTLOGUTILS_H
#define EVENTLOGUTILS_H

#include <QJsonObject>
#include <string>

class EventLogUtils
{
public:
    enum EventTID {
        OpeningTime     = 1000000000,
        ClosingTime     = 1000000001,
        Start           = 1000200030,
        StartPhoto      = 1000200031,
        StartRecording  = 1000200032,
        EndRecording    = 1000200033,
    };

    static EventLogUtils &get();
    void writeLogs(QJsonObject &data);

private:
    bool (*init)(const std::string &packagename, bool enable_sig) = nullptr;
    void (*writeEventLog)(const std::string &eventdata) = nullptr;

    static EventLogUtils *m_instance;

    EventLogUtils();
};

#endif // EVENTLOGUTILS_H
