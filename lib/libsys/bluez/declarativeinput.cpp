/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "declarativeinput.h"

DeclarativeInput::DeclarativeInput(const BluezQt::InputPtr &input, QObject *parent)
    : QObject(parent)
    , m_input(input)
{
    connect(m_input.data(), &BluezQt::Input::reconnectModeChanged, this, &DeclarativeInput::reconnectModeChanged);
}

BluezQt::Input::ReconnectMode DeclarativeInput::reconnectMode() const
{
    return m_input->reconnectMode();
}
