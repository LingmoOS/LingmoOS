// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

#pragma once

#include <QVariant>

#include "quickdialog.h"

class DynamicLauncherDialog : public QuickDialog
{
    Q_OBJECT
public:
    explicit DynamicLauncherDialog(const QString &title, const QIcon &icon, const QString &name, const QUrl &launcherURL, QObject *parent = nullptr);

    Q_PROPERTY(QString name MEMBER m_name NOTIFY nameChanged)
    Q_SIGNAL void nameChanged();
    QString m_name;

    Q_PROPERTY(QVariant icon MEMBER m_icon NOTIFY iconChanged)
    Q_SIGNAL void iconChanged();
    QVariant m_icon;
};
