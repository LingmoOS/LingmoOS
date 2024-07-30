/*
 *  SPDX-FileCopyrightText: 2023 Oliver Beard <olib141@outlook.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QObject>
#include <qqmlregistration.h>

class ApplicationInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString desktopName MEMBER m_desktopName WRITE setDesktopName NOTIFY desktopNameChanged)
    Q_PROPERTY(bool exists MEMBER m_exists NOTIFY existsChanged)
    Q_PROPERTY(QString name MEMBER m_name NOTIFY nameChanged)
    Q_PROPERTY(QString icon MEMBER m_icon NOTIFY iconChanged)
    QML_ELEMENT

public:
    void setDesktopName(const QString &desktopName);

Q_SIGNALS:
    void desktopNameChanged();
    void existsChanged();
    void nameChanged();
    void iconChanged();

private:
    QString m_desktopName;
    bool m_exists;
    QString m_name;
    QString m_icon;
};
