// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <DConfig>

DCORE_USE_NAMESPACE

namespace apps {
class AppsDockedHelper : public QObject
{
    Q_OBJECT

public:
    static AppsDockedHelper* instance();

    bool isDocked(const QString &appId) const;
    void setDocked(const QString &appId, bool docked);

private:
    AppsDockedHelper(QObject *parent = nullptr);

private:
    DConfig* m_config;
    QSet<QString> m_dockedDesktopIDs;
};
}
