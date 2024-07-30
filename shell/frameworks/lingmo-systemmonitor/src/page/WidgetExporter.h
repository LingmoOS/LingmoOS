/*
 * SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#pragma once

#include <QObject>
#include <qqmlregistration.h>

class FaceLoader;
class KConfigGroup;

class WidgetExporter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool lingmoshellAvailable MEMBER m_lingmoshellAvailable NOTIFY lingmoshellAvailableChanged)
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit WidgetExporter(QObject *parent = nullptr);

    Q_INVOKABLE void exportAsWidget(FaceLoader *loader) const;
Q_SIGNALS:
    void lingmoshellAvailableChanged();

private:
    bool m_lingmoshellAvailable;
    QString configEntriesScript(const KConfigGroup &group, const QStringList &path = {}) const;
};
