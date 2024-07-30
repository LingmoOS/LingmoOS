/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 Aurélien Gâteau <agateau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef KMESSAGEBOX_P_H
#define KMESSAGEBOX_P_H

#include <kmessageboxdontaskagaininterface.h>
#include <kmessageboxnotifyinterface.h>

namespace KMessageBox
{
KMessageBoxDontAskAgainInterface *dontAskAgainInterface();

KMessageBoxNotifyInterface *notifyInterface();

bool isNotifyInterfaceLoaded();

} // KMessageBox

#endif /* KMESSAGEBOX_P_H */
