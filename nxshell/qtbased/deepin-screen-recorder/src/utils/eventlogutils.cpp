// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QLibrary>
#include <QDir>
#include <QLibraryInfo>
#include <QJsonDocument>

#include "eventlogutils.h"

EventLogUtils *EventLogUtils::m_instance(nullptr);

EventLogUtils &EventLogUtils::get()
{
    if (m_instance == nullptr) {
        m_instance = new EventLogUtils;
    }
    return *m_instance;
}

EventLogUtils::EventLogUtils()
{
    QLibrary library("libdeepin-event-log.so");

    init = reinterpret_cast<bool (*)(const std::string &, bool)>(library.resolve("Initialize"));
    writeEventLog = reinterpret_cast<void (*)(const std::string &)>(library.resolve("WriteEventLog"));

    if (init == nullptr)
        return;

    init("deepin-screen-recorder", true);
}

void EventLogUtils::writeLogs(QJsonObject &data)
{
    if (writeEventLog == nullptr)
        return;

    //std::string str = QJsonDocument(data).toJson(QJsonDocument::Compact).toStdString();
    writeEventLog(QJsonDocument(data).toJson(QJsonDocument::Compact).toStdString());
}
