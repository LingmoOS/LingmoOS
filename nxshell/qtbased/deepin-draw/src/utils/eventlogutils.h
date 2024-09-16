// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EVENTLOGUTILS_H
#define EVENTLOGUTILS_H
#include <QJsonObject>
#include <string>

class Eventlogutils
{
public:
    enum EventTID {
        OpenTime     = 1000000000,
        CloseTime     = 1000000001,
        StartUp           = 1000000003,
        Quit            = 1000000004,
    };

    static Eventlogutils *GetInstance();
    void writeLogs(QJsonObject &data);
private :
    static Eventlogutils *m_pInstance;
    Eventlogutils();

    bool (*initFunc)(const std::string &packagename, bool enable_sig) = nullptr;
    void (*writeEventLogFunc)(const std::string &eventdata) = nullptr;
};

#endif // EVENTLOGUTILS_H
