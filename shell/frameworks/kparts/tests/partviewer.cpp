/*
    SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2000 Simon Hausmann <hausmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "partviewer.h"

#include <KActionCollection>
#include <KActionMenu>
#include <KLocalizedString>
#include <KPluginFactory>
#include <partloader.h>

#include <QAction>
#include <QApplication>
#include <QFile>
#include <QFileDialog>
#include <QMimeDatabase>
#include <QTest>

PartViewer::PartViewer()
{
    setXMLFile(QFINDTESTDATA("partviewer_shell.rc"));

    QAction *paOpen = new QAction(QIcon::fromTheme(QStringLiteral("document-open")), QStringLiteral("&Open file"), this);
    actionCollection()->addAction(QStringLiteral("file_open"), paOpen);
    connect(paOpen, &QAction::triggered, this, &PartViewer::slotFileOpen);

    QAction *paQuit = new QAction(QIcon::fromTheme(QStringLiteral("application-exit")), QStringLiteral("&Quit"), this);
    actionCollection()->addAction(QStringLiteral("file_quit"), paQuit);
    connect(paQuit, &QAction::triggered, this, &PartViewer::close);

    m_part = nullptr;

    // Set a reasonable size
    resize(600, 350);
}

PartViewer::~PartViewer()
{
    qDeleteAll(m_openWithActions);
    delete m_part;
}

void PartViewer::switchToPart(const QUrl &url)
{
    setCentralWidget(m_part->widget());
    // Integrate its GUI
    createGUI(m_part);

    m_part->openUrl(url);
}

void PartViewer::openUrl(const QUrl &url)
{
    unplugActionList(QStringLiteral("file_openwith"));
    delete m_part;
    QMimeDatabase db;
    const QString mimeType = db.mimeTypeForUrl(url).name();

    if (auto res = KParts::PartLoader::instantiatePartForMimeType<KParts::ReadOnlyPart>(mimeType, this, this)) {
        m_part = res.plugin;
        switchToPart(url);
    } else {
        qWarning() << res.errorString;
    }

    // Show available parts in the GUI

    qDeleteAll(m_openWithActions);
    m_openWithActions.clear();
    const QList<KPluginMetaData> plugins = KParts::PartLoader::partsForMimeType(mimeType);
    for (const KPluginMetaData &plugin : plugins) {
        QAction *action = new QAction(plugin.name(), this);
        connect(action, &QAction::triggered, this, [this, plugin, url] {
            loadPlugin(plugin, url);
        });
        m_openWithActions.append(action);
    }
    if (!m_openWithActions.isEmpty()) {
        QAction *sep = new QAction(this);
        sep->setSeparator(true);
        m_openWithActions.append(sep);
    }
    plugActionList(QStringLiteral("file_openwith"), m_openWithActions);
}

void PartViewer::slotFileOpen()
{
    QUrl url = QFileDialog::getOpenFileUrl();
    if (!url.isEmpty()) {
        openUrl(url);
    }
}

void PartViewer::loadPlugin(const KPluginMetaData &md, const QUrl &url)
{
    delete m_part;
    auto factory = KPluginFactory::loadFactory(md).plugin;
    Q_ASSERT(factory);
    m_part = factory->create<KParts::ReadOnlyPart>(this, this, QVariantList());
    if (m_part) {
        switchToPart(url);
    }
}

int main(int argc, char **argv)
{
    // This is a test window for showing any part

    QApplication app(argc, argv);
    PartViewer *shell = new PartViewer;
    if (argc > 1) {
        QUrl url = QUrl::fromUserInput(QLatin1String(argv[1]));
        shell->openUrl(url);
    } else {
        shell->slotFileOpen();
    }
    shell->show();
    return app.exec();
}

#include "moc_partviewer.cpp"
