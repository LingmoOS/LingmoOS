// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DETAILWIDGETMANAGER_H
#define DETAILWIDGETMANAGER_H

#include <QMutex>
#include <QReadWriteLock>
#include <QObject>

class DetailWidgetManager : public QObject
{
    Q_OBJECT
public:
    static DetailWidgetManager &getInstance();

    //公开的接口
    void jumpDetailWidget(const QString &msgCode);
    void jumpProcessWidget(const QString &msgCode);

signals:
    void sigJumpToDetailWidget(QString msgCode);
    void sigJumpToProcessWidget(QString msgCode);

private:
    DetailWidgetManager(QObject *parent = nullptr);
    ~DetailWidgetManager();
    DetailWidgetManager(const DetailWidgetManager &) = delete;
    DetailWidgetManager &operator=(const DetailWidgetManager &) = delete;

    QReadWriteLock internalMutex;

    static QMutex mutex;
    static QAtomicPointer<DetailWidgetManager> instance;
};

#endif // DETAILWIDGETMANAGER_H
