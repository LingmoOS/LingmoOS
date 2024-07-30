/*
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kquickcontrolsprivateplugin.h"

#include <QQmlEngine>

#include "keysequencehelper.h"
#include "keysequencevalidator.h"
#include "translationcontext.h"

void KQuickControlsPrivatePlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QString::fromLatin1(uri) == QLatin1String("org.kde.private.kquickcontrols"));
    qmlRegisterType<KeySequenceHelper>(uri, 2, 0, "KeySequenceHelper");
    qmlRegisterType<TranslationContext>(uri, 2, 0, "TranslationContext");
    qmlRegisterType<KeySequenceValidator>(uri, 2, 0, "KeySequenceValidator");
    // Register the Helper again publicly but uncreatable, so one can access the shortcuttype enum
    // values as for example "ShortcutType.StandardShortcuts" from qml
    qmlRegisterUncreatableType<KeySequenceHelper>("org.kde.kquickcontrols", 2, 0, "ShortcutType", QStringLiteral("This is just to allow accessing the enum"));
}

#include "moc_kquickcontrolsprivateplugin.cpp"
