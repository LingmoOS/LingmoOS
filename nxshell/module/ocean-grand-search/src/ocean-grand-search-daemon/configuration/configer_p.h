// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONFIGER_P_H
#define CONFIGER_P_H

#include "configuration/configer.h"
#include "userpreference.h"

#include <QPointer>
#include <QReadWriteLock>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QSettings>

namespace GrandSearch {

class ConfigerPrivate
{
    friend class Configer;
public:
    ConfigerPrivate(Configer *parent);
    static UserPreferencePointer defaultSearcher();
    static UserPreferencePointer fileSearcher();
    static UserPreferencePointer tailerData();
    static UserPreferencePointer blacklist();
    static UserPreferencePointer webSearchEngine();
    static UserPreferencePointer semanticEngine();
    bool updateConfig1(QSettings *);
    void resetPath(QString &path) const;
private:
    Configer *q;
    mutable QReadWriteLock m_rwLock;
    UserPreferencePointer m_root;
    QString m_configPath;
    QFileSystemWatcher *m_watcher = nullptr;
    QTimer m_delayLoad;
};

}

#endif // CONFIGER_P_H
