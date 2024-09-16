// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DESKTOPFILEGENERATOR_H
#define DESKTOPFILEGENERATOR_H

#include "desktopentry.h"

struct DesktopFileGenerator
{
    static QString generate(const QVariantMap &desktopFile, QString &err) noexcept;

private:
    static bool checkValidation(const QVariantMap &desktopFile, QString &err) noexcept;
    static bool processMainGroup(DesktopEntry::container_type &content, const QVariantMap &rawValue) noexcept;
    static bool
    processActionGroup(QStringList actions, DesktopEntry::container_type &content, const QVariantMap &rawValue) noexcept;
    static int processMainGroupLocaleEntry(DesktopEntry::container_type::iterator mainEntry,
                                           const QString &key,
                                           const QVariant &value) noexcept;
};

#endif
