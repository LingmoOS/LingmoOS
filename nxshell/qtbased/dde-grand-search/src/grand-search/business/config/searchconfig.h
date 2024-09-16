// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHCONFIG_H
#define SEARCHCONFIG_H

#include <QObject>
#include <QSettings>
#include <QMutex>

namespace GrandSearch {

class SearchConfig : public QObject
{
    Q_OBJECT
public:
    static SearchConfig *instance();

    QVariant getConfig(const QString &group, const QString &key, const QVariant &defaultValue = QVariant());
    void setConfig(const QString &group, const QString &key, const QVariant &value);
    void removeConfig(const QString &group, const QString &key);
    void setConfigList(const QString &group, const QStringList &keys, const QVariantList &values);
    void removeConfigList(const QString &group, const QStringList &keys);

protected:
    explicit SearchConfig();
    ~SearchConfig();

private:
    void convertConfigFile();

private:
    QMutex  m_mutex;
    QSettings *m_settings = nullptr;
};

}

#endif // SEARCHCONFIG_H
