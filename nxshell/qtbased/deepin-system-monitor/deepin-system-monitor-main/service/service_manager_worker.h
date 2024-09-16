// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SERVICE_MANAGER_WORKER_H
#define SERVICE_MANAGER_WORKER_H

#include <QObject>
#include <QList>

class SystemServiceEntry;

class ServiceManagerWorker : public QObject
{
    Q_OBJECT
public:
    explicit ServiceManagerWorker(QObject *parent = nullptr);

Q_SIGNALS:
    void resultReady(const QList<SystemServiceEntry> list);

public Q_SLOTS:
    void startJob();

private:
    inline static QString readUnitDescriptionFromUnitFile(const QString &path);
};

#endif // SERVICE_MANAGER_WORKER_H
