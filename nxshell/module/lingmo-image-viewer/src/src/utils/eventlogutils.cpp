// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "eventlogutils.h"
#include <QLibrary>
#include <QDir>
#include <QLibraryInfo>
#include <QJsonDocument>

Eventlogutils *Eventlogutils::m_pInstance = nullptr;
Eventlogutils *Eventlogutils::GetInstance()
{
    if (m_pInstance == nullptr) {
        m_pInstance  = new Eventlogutils();
    }
    return m_pInstance;
}

void Eventlogutils::writeLogs(QJsonObject &data)
{
    if (!writeEventLogFunc)
        return;

    writeEventLogFunc(QJsonDocument(data).toJson(QJsonDocument::Compact).toStdString());
}

Eventlogutils::Eventlogutils()
{
    QLibrary library("liblingmo-event-log.so");
    initFunc = reinterpret_cast<bool (*)(const std::string &, bool)>(library.resolve("Initialize"));
    writeEventLogFunc = reinterpret_cast<void (*)(const std::string &)>(library.resolve("WriteEventLog"));

    if (!initFunc)
        return;

    initFunc("lingmo-image-viewer", true);
}
