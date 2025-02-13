/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef DECLARATIVEINPUT_H
#define DECLARATIVEINPUT_H

#include <BluezQt/Input>

class DeclarativeInput : public QObject
{
    Q_OBJECT
    Q_PROPERTY(BluezQt::Input::ReconnectMode reconnectMode READ reconnectMode NOTIFY reconnectModeChanged)

public:
    explicit DeclarativeInput(const BluezQt::InputPtr &input, QObject *parent = nullptr);

    BluezQt::Input::ReconnectMode reconnectMode() const;

Q_SIGNALS:
    void reconnectModeChanged(BluezQt::Input::ReconnectMode mode);

private:
    BluezQt::InputPtr m_input;
};

#endif // DECLARATIVEINPUT_H
