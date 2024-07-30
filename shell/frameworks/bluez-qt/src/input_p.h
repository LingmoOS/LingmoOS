/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_INPUT_P_H
#define BLUEZQT_INPUT_P_H

#include <QObject>

#include "input.h"

namespace BluezQt
{
class InputPrivate : public QObject
{
    Q_OBJECT

public:
    explicit InputPrivate(const QString &path, const QVariantMap &properties);

    void propertiesChanged(const QString &interface, const QVariantMap &changed, const QStringList &invalidated);

    QWeakPointer<Input> q;

    QString m_path;
    Input::ReconnectMode m_reconnectMode;
};

} // namespace BluezQt

#endif // BLUEZQT_INPUT_P_H
