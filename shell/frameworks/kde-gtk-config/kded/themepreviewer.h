/*
 * SPDX-FileCopyrightText: 2019 Mikhail Zolotukhin <zomial@protonmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QProcess>
#include <QString>

class ThemePreviewer : QObject
{
    Q_OBJECT
public:
    ThemePreviewer(QObject *parent);

    void showGtk3App(const QString &themeName);

private:
    static const QString gtk3PreviewerExecutablePath;

    QProcess gtk3PreviewerProccess;
};
