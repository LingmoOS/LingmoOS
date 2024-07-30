/*
    SPDX-FileCopyrightText: 2014 Bhushan Shah <bhush94@gmail.com>
    SPDX-FileCopyrightText: 2014 Marco Martin <notmart@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "lingmowindowedcorona.h"
#include "lingmowindowedview.h"
#include <QAction>
#include <QCommandLineParser>
#include <QDebug>

#include <KActionCollection>
#include <KLocalizedString>

#include <KPackage/Package>
#include <KPackage/PackageLoader>
#include <Lingmo/PluginLoader>

using namespace Qt::StringLiterals;

LingmoWindowedCorona::LingmoWindowedCorona(const QString &shell, QObject *parent)
    : Lingmo::Corona(parent)
{
    KPackage::Package package = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Lingmo/Shell"));
    package.setPath(shell);
    setKPackage(package);
    // QMetaObject::invokeMethod(this, "load", Qt::QueuedConnection);
    load();
}

void LingmoWindowedCorona::loadApplet(const QString &applet, const QVariantList &arguments)
{
    if (containments().isEmpty()) {
        return;
    }

    Lingmo::Containment *cont = containments().first();

    // forbid more instances per applet (todo: activate the corresponding already loaded applet)
    for (Lingmo::Applet *a : cont->applets()) {
        if (a->pluginMetaData().pluginId() == applet) {
            return;
        }
    }
    m_view = new LingmoWindowedView();
    m_view->setHasStatusNotifier(m_hasStatusNotifier);
    m_view->show();

    KConfigGroup appletsGroup(KSharedConfig::openConfig(), QStringLiteral("Applets"));
    QString plugin;
    for (const QString &group : appletsGroup.groupList()) {
        KConfigGroup cg(&appletsGroup, group);
        plugin = cg.readEntry("plugin", QString());

        if (plugin == applet) {
            Lingmo::Applet *a = Lingmo::PluginLoader::self()->loadApplet(applet, group.toInt(), arguments);
            if (!a) {
                qWarning() << "Unable to load applet" << applet << "with arguments" << arguments;
                delete m_view;
                m_view = nullptr;
                return;
            }
            a->restore(cg);

            // Access a->config() before adding to containment
            // will cause applets to be saved in palsmawindowedrc
            // so applets will only be created on demand
            KConfigGroup cg2 = a->config();
            cont->addApplet(a);

            m_view->setApplet(a);
            return;
        }
    }

    Lingmo::Applet *a = Lingmo::PluginLoader::self()->loadApplet(applet, 0, arguments);
    if (!a) {
        qWarning() << "Unable to load applet" << applet << "with arguments" << arguments;
        delete m_view;
        m_view = nullptr;
        return;
    }

    // Access a->config() before adding to containment
    // will cause applets to be saved in palsmawindowedrc
    // so applets will only be created on demand
    KConfigGroup cg2 = a->config();
    cont->addApplet(a);

    m_view->setApplet(a);
}

void LingmoWindowedCorona::activateRequested(const QStringList &arguments, const QString &workingDirectory)
{
    Q_UNUSED(workingDirectory)
    if (arguments.count() <= 1) {
        return;
    }

    QCommandLineParser parser;
    parser.setApplicationDescription(i18n("Lingmo Windowed"));
    parser.addOption(
        QCommandLineOption(QStringLiteral("statusnotifier"), i18n("Makes the plasmoid stay alive in the Notification Area, even when the window is closed.")));
    parser.addPositionalArgument(QStringLiteral("applet"), i18n("The applet to open."));
    parser.addPositionalArgument(QStringLiteral("args"), i18n("Arguments to pass to the plasmoid."), QStringLiteral("[args...]"));
    QCommandLineOption configOpt(u"config"_s);
    parser.addOption(configOpt);
    parser.addVersionOption();
    parser.addHelpOption();
    parser.process(arguments);

    if (parser.isSet(configOpt)) {
        m_view->showConfigurationInterface();
        return;
    }

    if (parser.positionalArguments().isEmpty()) {
        parser.showHelp(1);
    }

    const QStringList positionalArguments = parser.positionalArguments();

    QVariantList args;
    QStringList::const_iterator constIterator = positionalArguments.constBegin() + 1;
    for (; constIterator != positionalArguments.constEnd(); ++constIterator) {
        args << (*constIterator);
    }

    loadApplet(positionalArguments.first(), args);
}

QRect LingmoWindowedCorona::screenGeometry(int id) const
{
    Q_UNUSED(id);
    // TODO?
    return QRect();
}

void LingmoWindowedCorona::load()
{
    /*this won't load applets, since applets are in lingmowindowedrc*/
    loadLayout(QStringLiteral("lingmowindowed-appletsrc"));

    bool found = false;
    for (auto c : containments()) {
        if (c->containmentType() == Lingmo::Containment::Desktop) {
            found = true;
            break;
        }
    }

    if (!found) {
        qDebug() << "Loading default layout";
        createContainment(QStringLiteral("empty"));
        saveLayout(QStringLiteral("lingmowindowed-appletsrc"));
    }

    for (auto c : containments()) {
        if (c->containmentType() == Lingmo::Containment::Desktop) {
            m_containment = c;
            m_containment->setFormFactor(Lingmo::Types::Application);
            QAction *removeAction = c->internalAction(QStringLiteral("remove"));
            if (removeAction) {
                removeAction->deleteLater();
            }
            break;
        }
    }
}

void LingmoWindowedCorona::setHasStatusNotifier(bool stay)
{
    m_hasStatusNotifier = stay;
}
