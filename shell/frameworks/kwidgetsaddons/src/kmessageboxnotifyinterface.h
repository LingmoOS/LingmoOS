/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 Aurélien Gâteau <agateau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KMESSAGEBOXNOTIFYINTERFACE_H
#define KMESSAGEBOXNOTIFYINTERFACE_H

#include "kmessagebox.h"
#include <qmetatype.h>

/**
 * @internal
 * Used internally by KMessageBox, implemented by plugin, for dynamic dependency on KNotification.
 */
class KMessageBoxNotifyInterface
{
public:
    KMessageBoxNotifyInterface()
    {
    }
    virtual ~KMessageBoxNotifyInterface()
    {
    }

    virtual void sendNotification(QMessageBox::Icon notificationType, const QString &message, QWidget *parent) = 0;
};

Q_DECLARE_METATYPE(KMessageBoxNotifyInterface *)

#define KMESSAGEBOXNOTIFY_PROPERTY "KMessageBoxNotify"

#endif
