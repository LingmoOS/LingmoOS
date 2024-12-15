// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
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
        OpeningTime         = 1000000000,
        ClosingTime         = 1000000001,
        Start               = 1000200000,
        StartRecording      = 1000200001,
        StartScreenShot     = 1000200002,
        StartOcr            = 1000200003,
        StartScrollShot     = 1000200004,
        EndRecording        = 1000200005,
        EndScreenShot       = 1000200006
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
