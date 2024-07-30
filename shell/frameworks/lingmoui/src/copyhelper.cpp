/*
 *  SPDX-FileCopyrightText: 2009 Alan Alpert <alan.alpert@nokia.com>
 *  SPDX-FileCopyrightText: 2010 Ménard Alexis <menard@kde.org>
 *  SPDX-FileCopyrightText: 2010 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "copyhelper.h"

#include <QClipboard>
#include <QGuiApplication>

void CopyHelperPrivate::copyTextToClipboard(const QString &text)
{
    qGuiApp->clipboard()->setText(text);
}
