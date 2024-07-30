/*
 *   SPDX-FileCopyrightText: 2010 Alejandro Fiestas Olivares <alex@eyeos.org>
 *   SPDX-FileCopyrightText: 2010 Eduardo Robles Elvira <edulix@gmail.com>
 *   SPDX-FileCopyrightText: 2010 UFO Coders <info@ufocoders.com>
 *   SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QObject>

#include <BluezQt/Device>

namespace Ui
{
class DialogWidget;
}

class KNotification;

class RequestPin : public QObject
{
    Q_OBJECT

public:
    explicit RequestPin(BluezQt::DevicePtr device, bool numeric = false, QObject *parent = nullptr);

Q_SIGNALS:
    void done(const QString &result);

private Q_SLOTS:
    void introducePin();
    void quit();

    void checkPin(const QString &pin);
    void dialogFinished(int result);

private:
    Ui::DialogWidget *m_dialogWidget = nullptr;
    KNotification *m_notification = nullptr;
    BluezQt::DevicePtr m_device;
    bool m_numeric;
};
