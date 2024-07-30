/*
    SPDX-FileCopyrightText: 2016 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2023 Natalie Clarius <natalie.clarius@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QDBusContext>
#include <QMap>
#include <QObject>
#include <QString>
#include <QTimer>

namespace PowerDevil
{
class ConfigOperation;
class Osd;

class OsdManager : public QObject, public QDBusContext
{
    Q_OBJECT

public:
    explicit OsdManager(QObject *parent = nullptr);

public Q_SLOTS:
    void hideOsd() const;
    void showOsd();
    void applyProfile(const QString &profile);

private:
    void quit();
    QTimer *m_cleanupTimer = new QTimer(this);
    PowerDevil::Osd *m_osd = nullptr;
};

} // namespace PowerDevil
