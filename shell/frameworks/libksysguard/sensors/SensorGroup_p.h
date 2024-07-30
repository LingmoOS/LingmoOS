/*
    SPDX-FileCopyrightText: 2020 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QHash>
#include <QString>

namespace KSysGuard
{
class SensorGroup
{
public:
    SensorGroup();
    ~SensorGroup();

    void retranslate();

    QString groupRegexForId(const QString &key);
    QString sensorNameForRegEx(const QString &expr);
    QString segmentNameForRegEx(const QString &expr);

private:
    QHash<QString, QString> m_sensorNames;
    QHash<QString, QString> m_segmentNames;
};
} // namespace KSysGuard
