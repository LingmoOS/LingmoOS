/*
 * SPDX-FileCopyrightText: 2020 Mikhail Zolotukhin <zomial@protonmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include <QDir>
#include <QRegularExpression>
#include <QString>
#include <QVariant>

#include "config_editor/gsettings.h"
#include "config_editor/gtk2.h"
#include "config_editor/settings_ini.h"
#include "config_editor/xsettings.h"

QString gtk2Theme();
void upgradeGtk2Theme();
void upgradeGtk3Theme();

int main()
{
    upgradeGtk2Theme();
    upgradeGtk3Theme();
    return 0;
}

void upgradeGtk2Theme()
{
    QString currentGtk2Theme = gtk2Theme();
    if (currentGtk2Theme.isEmpty() //
        || currentGtk2Theme == QStringLiteral("oxygen-gtk") //
        || currentGtk2Theme == QStringLiteral("BreezyGTK") //
        || currentGtk2Theme == QStringLiteral("Orion")) {
        Gtk2ConfigEditor::setValue(QStringLiteral("gtk-theme-name"), QStringLiteral("Breeze"));
    }
}

void upgradeGtk3Theme()
{
    QString currentGtk3Theme = SettingsIniEditor::value(QStringLiteral("gtk-theme-name"), 3);
    if (currentGtk3Theme.isEmpty() //
        || currentGtk3Theme == QStringLiteral("oxygen-gtk") //
        || currentGtk3Theme == QStringLiteral("BreezyGTK") //
        || currentGtk3Theme == QStringLiteral("Orion")) {
        GSettingsEditor::setValue("gtk-theme", QStringLiteral("Breeze"));
        SettingsIniEditor::setValue(QStringLiteral("gtk-theme-name"), QStringLiteral("Breeze"), 3);
        XSettingsEditor::setValue(QStringLiteral("Net/ThemeName"), QStringLiteral("Breeze"));
    }
}

QString gtk2Theme()
{
    QString gtkrcPath = QDir::homePath() + QStringLiteral("/.gtkrc-2.0");
    QFile gtkrc(gtkrcPath);
    if (gtkrc.open(QIODevice::ReadWrite | QIODevice::Text)) {
        const QRegularExpression regExp(QStringLiteral("gtk-theme-name=[^\n]*($|\n)"));
        while (!gtkrc.atEnd()) {
            QString line = gtkrc.readLine();
            if (line.contains(regExp)) {
                return line.split('"')[1];
            }
        }
    }

    return QString();
}
