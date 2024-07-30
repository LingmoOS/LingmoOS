/*
    SPDX-FileCopyrightText: 2019 Filip Fila <filipfila.kde@gmail.com>
    SPDX-FileCopyrightText: 2013 Reza Fatahilah Shah <rshah0385@kireihana.com>

    SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef SDDMAUTHHELPER_H
#define SDDMAUTHHELPER_H

#include <KAuth/ActionReply>
#include <KAuth/HelperSupport>
#include <QObject>

using namespace KAuth;

class SddmAuthHelper : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    ActionReply sync(const QVariantMap &args);
    ActionReply reset(const QVariantMap &args);
    ActionReply save(const QVariantMap &args);
    ActionReply installtheme(const QVariantMap &args);
    ActionReply uninstalltheme(const QVariantMap &args);

public:
    static void copyFile(const QString &source, const QString &destination);
    static void copyDirectoryRecursively(const QString &source, const QString &destination, QSet<QString> &done);
};

#endif // SDDMAUTHHELPER_H
