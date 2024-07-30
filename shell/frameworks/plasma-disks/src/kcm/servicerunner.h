// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#pragma once

#include <KService>
#include <QObject>

class ServiceRunner : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(bool canRun READ canRun NOTIFY canRunChanged)

public:
    Q_INVOKABLE void run();

    QString name() const;
    void setName(const QString &name);

    bool canRun() const;

Q_SIGNALS:
    void nameChanged();
    void canRunChanged();

private:
    QString m_name;
    KService::Ptr m_service{nullptr};
};
