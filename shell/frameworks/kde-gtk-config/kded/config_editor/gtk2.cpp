// SPDX-FileCopyrightText: 2019, 2022 Mikhail Zolotukhin <zomial@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "gtk2.h"

#include <QDir>
#include <QRegularExpression>

#include "config_editor/utils.h"

namespace Gtk2ConfigEditor
{
namespace
{

void replaceValueInGtkrcContents(QString &gtkrcContents, const QString &paramName, const QVariant &paramValue)
{
    const QRegularExpression regExp(paramName + QStringLiteral("=[^\n]*($|\n)"));

    QString newConfigString;
    if (paramValue.type() == QVariant::Type::String) {
        newConfigString = QStringLiteral("%1=\"%2\"\n").arg(paramName, paramValue.toString());
    } else if (paramValue.type() == QVariant::Type::Bool) {
        // GTK2 does not support 'true' and 'false' as values
        newConfigString = QStringLiteral("%1=%2\n").arg(paramName, QString::number(paramValue.toInt()));
    } else {
        newConfigString = QStringLiteral("%1=%2\n").arg(paramName, paramValue.toString());
    }

    if (gtkrcContents.contains(regExp)) {
        gtkrcContents.replace(regExp, newConfigString);
    } else {
        gtkrcContents = newConfigString + gtkrcContents;
    }
}
}

void setValue(const QString &paramName, const QVariant &paramValue)
{
    QString gtkrcPath = qEnvironmentVariable("GTK2_RC_FILES", QDir::homePath() + QStringLiteral("/.gtkrc-2.0"));
    if (gtkrcPath.contains(QStringLiteral(":/"))) { // I.e. env variable contains multiple paths
        gtkrcPath = QDir::homePath() + QStringLiteral("/.gtkrc-2.0");
    }
    QFile gtkrc(gtkrcPath);
    QString gtkrcContents = Utils::readFileContents(gtkrc);
    replaceValueInGtkrcContents(gtkrcContents, paramName, paramValue);
    gtkrc.remove();
    gtkrc.open(QIODevice::WriteOnly | QIODevice::Text);
    gtkrc.write(gtkrcContents.toUtf8());
}

void removeLegacyStrings()
{
    QString gtkrcPath = QDir::homePath() + QStringLiteral("/.gtkrc-2.0");
    QFile gtkrc(gtkrcPath);
    QString gtkrcContents = Utils::readFileContents(gtkrc);
    if (gtkrcContents.isNull()) {
        return;
    }

    // Remove "include" lines
    // Example:
    // include "/usr/share/themes/Adwaita-dark/gtk-2.0/gtkrc"
    static const QRegularExpression includeRegExp(QStringLiteral("include .*\n"));
    gtkrcContents.remove(includeRegExp);

    // Remove redundant font config lines
    // Example:
    // style "user-font"
    // {
    //     font_name="Noto Sans Regular"
    // }
    // widget_class "*" style "user-font"
    static const QRegularExpression userFontStyleRegexp(QStringLiteral("style(.|\n)*{(.|\n)*}\nwidget_class.*\"user-font\""));
    gtkrcContents.remove(userFontStyleRegexp);

    gtkrc.remove();
    gtkrc.open(QIODevice::WriteOnly | QIODevice::Text);
    gtkrc.write(gtkrcContents.toUtf8());
}

}
