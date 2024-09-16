// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GSETTINGWATCHER_H
#define GSETTINGWATCHER_H

#include <QObject>
#include <QHash>

class QGSettings;
class GSettingWatcher : public QObject
{
    Q_OBJECT
public:
    static GSettingWatcher *instance();

    void bind(const QString &gsettingsName, QWidget *binder);
    void erase(const QString &gsettingsName);
    void erase(const QString &gsettingsName, QWidget *binder);
    const QString getStatus(const QString &gsettingsName);

signals:
    void enableChanged(const QString &gsettingsName, bool enable);

private:
    GSettingWatcher(QObject *parent = nullptr);

    void setStatus(const QString &gsettingsName, QWidget *binder);
    void onStatusModeChanged(const QString &key);

private:
    QMultiHash<QString, QWidget *> m_map;
    QGSettings *m_gsettings;
};

#endif // GSETTINGWATCHER_H
