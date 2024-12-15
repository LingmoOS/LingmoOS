// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dsglobal.h"

#include <QObject>
#include <DConfig>
#include <QScopedPointer>
#include <QJsonArray>

DCORE_USE_NAMESPACE


namespace dock {

class TaskManagerSettings : public QObject
{
    Q_OBJECT

public:
    static TaskManagerSettings* instance();

    bool isAllowedForceQuit();
    void setAllowedForceQuit(bool allowed);

    bool isWindowSplit();
    void setWindowSplit(bool split);

    void setDockedDesktopFiles(QJsonArray desktopfiles);
    void appnedDockedDesktopfiles(QJsonObject desktopfile);
    void removeDockedDesktopfile(QJsonObject desktopfile);
    QJsonArray dockedDesktopFiles();

private:
    explicit TaskManagerSettings(QObject *parent = nullptr);
    inline void dockedItemsPersisted();
    inline void loadDockedItems();

Q_SIGNALS:
    void allowedForceQuitChanged();
    void windowSplitChanged();
    void dockedItemsChanged();

private:
    DConfig* m_taskManagerDconfig;

    bool m_allowForceQuit;
    bool m_windowSplit;
    QJsonArray m_dockedItems;
};
}
