/*
    SPDX-FileCopyrightText: 2021 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "colorsapplicator.h"
#include "colorsmodel.h"
#include "colorssettings.h"

#include "../kcms-common_p.h"
#include "../krdb/krdb.h"

#include <KColorScheme>
#include <KConfig>
#include <KLocalizedString>

#include <QCommandLineParser>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDebug>
#include <QFile>
#include <QGuiApplication>
#include <QTimer>

int main(int argc, char **argv)
{
    // This is a CLI application, but we require at least a QGuiApplication to be able
    // to use QColor, so let's just roll with one of these
    QGuiApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("lingmo-apply-colorscheme"));
    QCoreApplication::setApplicationVersion(QStringLiteral("1.0"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
    KLocalizedString::setApplicationDomain("lingmo-apply-colorscheme");

    QCommandLineParser *parser = new QCommandLineParser;
    parser->addHelpOption();
    parser->setApplicationDescription(
        i18n("This tool allows you to set the color scheme for the current Plasma session, without accidentally setting it to one that is either not "
             "available, or which is already set."));
    parser->addPositionalArgument(
        QStringLiteral("colorscheme"),
        i18n("The name of the color scheme you wish to set for your current Plasma session (passing a full path will only use the last part of the path)"));

    const auto listSchemes = QStringList({QStringLiteral("list-schemes"), QStringLiteral("l")});
    parser->addOption(QCommandLineOption(listSchemes, i18n("Show all the color schemes available on the system (and which is the current theme)")));

    const auto accentColor = QStringList({QStringLiteral("accent-color"), QStringLiteral("a")});
    parser->addOption(
        QCommandLineOption(accentColor,
                           i18n("The name of the accent color you want to set. SVG color names (https://www.w3.org/TR/SVG11/types.html#ColorKeywords) and hex "
                                "color codes are supported. Quote the hex code if there is possibility of shell expansion"),
                           "accentColor",
                           "0"));
    parser->process(app);

    int exitCode{0};
    ColorsSettings *settings = new ColorsSettings(&app);
    QTextStream ts(stdout);
    ColorsModel *model = new ColorsModel(&app);
    model->load();
    model->setSelectedScheme(settings->colorScheme());
    if (!parser->positionalArguments().isEmpty() && !parser->isSet(QStringLiteral("accent-color"))) {
        QString requestedScheme{parser->positionalArguments().first()};
        const QString dirSplit{"/"};
        if (requestedScheme.contains(dirSplit)) {
            QStringList splitScheme = requestedScheme.split(dirSplit, Qt::SkipEmptyParts);
            requestedScheme = splitScheme.last();
            if (requestedScheme.endsWith(QStringLiteral(".colors"))) {
                requestedScheme = requestedScheme.left(requestedScheme.lastIndexOf(QStringLiteral(".")));
            } else {
                exitCode = -1;
            }
        }

        if (exitCode == 0) {
            if (settings->colorScheme() == requestedScheme) {
                ts << i18n("The requested theme \"%1\" is already set as the theme for the current Plasma session.", requestedScheme) << Qt::endl;
                // Not an error condition, no reason to set the theme, but basically this is fine
            } else if (!requestedScheme.isEmpty()) {
                int newSchemeIndex{-1};
                QStringList availableThemes;
                for (int i = 0; i < model->rowCount(QModelIndex()); ++i) {
                    QString schemeName = model->data(model->index(i, 0), ColorsModel::SchemeNameRole).toString();
                    availableThemes << schemeName;
                    if (schemeName == requestedScheme) {
                        newSchemeIndex = i;
                        // No breaking out, we're using the list of names if things fail, and
                        // it's not particularly expensive compared to what we've already done
                    }
                }

                if (newSchemeIndex > -1) {
                    model->setSelectedScheme(requestedScheme);
                    settings->setColorScheme(requestedScheme);
                    const QString path =
                        QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("color-schemes/%1.colors").arg(model->selectedScheme()));

                    auto msg = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"),
                                                      QStringLiteral("/org/kde/KWin/BlendChanges"),
                                                      QStringLiteral("org.kde.KWin.BlendChanges"),
                                                      QStringLiteral("start"));
                    msg << 300;
                    // This is deliberately blocking so that we ensure Kwin has processed the
                    // animation start event before we potentially trigger client side changes
                    QDBusConnection::sessionBus().call(msg);
                    
                    applyScheme(path, settings->config());
                    settings->save();
                    notifyKcmChange(GlobalChangeType::PaletteChanged);
                    ts << i18n("Successfully applied the color scheme %1 to your current Plasma session", requestedScheme) << Qt::endl;
                } else {
                    ts << i18n("Could not find theme \"%1\". The theme should be one of the following options: %2",
                               requestedScheme,
                               availableThemes.join(QLatin1String{", "}))
                       << Qt::endl;
                }
            } else {
                // This shouldn't happen, but let's catch it and make angry noises, just in case...
                ts << i18n("You have managed to pass an empty color scheme name, which isn't supported behavior.") << Qt::endl;
                exitCode = -1;
            }
        } else {
            ts << i18n("The file you attempted to set as your scheme, %1, could not be identified as a color scheme.", parser->positionalArguments().first())
               << Qt::endl;
            exitCode = -1;
        }
    } else if (parser->isSet(QStringLiteral("accent-color"))) {
        QString accentColor = parser->value("accent-color");

        if (QColor::isValidColor(accentColor)) {
            const QString path =
                QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("color-schemes/%1.colors").arg(settings->colorScheme()));

            auto msg = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"),
                                                      QStringLiteral("/org/kde/KWin/BlendChanges"),
                                                      QStringLiteral("org.kde.KWin.BlendChanges"),
                                                      QStringLiteral("start"));
            msg << 300;
            // This is deliberately blocking so that we ensure Kwin has processed the
            // animation start event before we potentially trigger client side changes
            QDBusConnection::sessionBus().call(msg);

            applyScheme(path, settings->config(), KConfig::Notify, {accentColor});
            notifyKcmChange(GlobalChangeType::PaletteChanged);

            ts << i18n("Successfully applied the accent color %1", accentColor) << Qt::endl;
        } else {
            ts << i18n("Invalid accent color ") << accentColor << Qt::endl;
            exitCode = -1;
        }

    } else if (parser->isSet(QStringLiteral("list-schemes"))) {
        ts << i18n("You have the following color schemes on your system:") << Qt::endl;
        int currentThemeIndex = model->selectedSchemeIndex();
        for (int i = 0; i < model->rowCount(QModelIndex()); ++i) {
            const QString schemeName = model->data(model->index(i, 0), ColorsModel::SchemeNameRole).toString();
            if (i == currentThemeIndex) {
                ts << i18n(" * %1 (current color scheme)", schemeName) << Qt::endl;
            } else {
                ts << QString(" * %1").arg(schemeName) << Qt::endl;
            }
        }
    } else {
        parser->showHelp();
    }
    QTimer::singleShot(0, &app, [&app, &exitCode]() {
        app.exit(exitCode);
    });

    return app.exec();
}
