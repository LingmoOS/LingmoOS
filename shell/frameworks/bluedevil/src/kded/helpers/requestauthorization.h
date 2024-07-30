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

class RequestAuthorization : public QObject
{
    Q_OBJECT

public:
    enum Result {
        Deny,
        Accept,
        AcceptAndTrust,
    };

    explicit RequestAuthorization(BluezQt::DevicePtr device, QObject *parent = nullptr);

Q_SIGNALS:
    void done(Result result);

private Q_SLOTS:
    void authorizeAndTrust();
    void authorize();
    void deny();

private:
    BluezQt::DevicePtr m_device;
};
