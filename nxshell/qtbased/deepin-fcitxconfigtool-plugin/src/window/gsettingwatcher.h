// Copyright (C) 2020 ~ 2021 Uniontech Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GSETTINGWATCHERR_H
#define GSETTINGWATCHERR_H
#include <QObject>
#include <QHash>
#include <QMap>

class QGSettings;
class QListView;
class QStandardItem;
class GSettingWatcher : public QObject
{
    Q_OBJECT
public:
    static GSettingWatcher *instance();

    void bind(const QString &gsettingsName, QWidget *binder);
    void bind(const QString &gsettingsName, QListView *viewer, QStandardItem *item);
    void erase(const QString &gsettingsName);
    void erase(const QString &gsettingsName, QWidget *binder);
    void insertState(const QString &);
    const QString getStatus(const QString &gsettingsName);
    QMap<QString, bool> getMenuState();

private:
    GSettingWatcher(QObject *parent = nullptr);

    void setStatus(const QString &gsettingsName, QWidget *binder);
    void setStatus(const QString &gsettingsName, QListView *viewer, QStandardItem *item);
    void onStatusModeChanged(const QString &key);

Q_SIGNALS:
    void requestUpdateSecondMenu(int);
    void requestUpdateSearchMenu(const QString &, bool);

private:
    QMultiHash<QString, QWidget *> m_map;
    QGSettings *m_gsettings;
    QHash<QString, QPair<QListView *, QStandardItem *>> m_menuMap;
    QMap<QString, bool> m_menuState;
};

#endif // GSETTINGWATCHER_H
