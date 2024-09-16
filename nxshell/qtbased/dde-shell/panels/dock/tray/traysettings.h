// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QScopedPointer>

#include <DConfig>

DCORE_USE_NAMESPACE

namespace dock {

class TraySettings : public QObject
{
    Q_OBJECT

public:
    static TraySettings* instance();

    bool trayItemIsOnDock(const QString &surfaceIds);
    void addTrayItemOnDock(const QString &surfaceIds);
    void removeTrayItemOnDock(const QString &surfaceIds);

private:
    explicit TraySettings(QObject *parent = nullptr);
    void init();

Q_SIGNALS:
    void hiddenSurfaceIdsChanged(const QStringList &hiddenSurfaceIds);

private:
    QScopedPointer<DConfig> m_trayConfig;
    QStringList m_hiddenSurfaceIds;
};
}
