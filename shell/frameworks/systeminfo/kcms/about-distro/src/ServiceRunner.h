// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020-2021 Harald Sitter <sitter@kde.org>

#pragma once

#include <KService>
#include <QObject>

class ServiceRunner : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString desktopFileName READ desktopFileName WRITE setDesktopFileName NOTIFY desktopFileNameChanged)
    Q_PROPERTY(QString genericName READ genericName NOTIFY changed)
    Q_PROPERTY(QString iconName READ iconName NOTIFY changed)
    Q_PROPERTY(bool canRun READ canRun NOTIFY changed)
public:
    using QObject::QObject;

    Q_INVOKABLE void run();

    QString desktopFileName() const;
    void setDesktopFileName(const QString &name);

    QString genericName() const;
    QString iconName() const;
    bool canRun() const;

Q_SIGNALS:
    void desktopFileNameChanged();
    void changed();

private:
    QString m_desktopFileName;
    QString m_genericName;
    QString m_iconName;
    KService::Ptr m_service{nullptr};
};
