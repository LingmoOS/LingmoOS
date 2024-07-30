/*
    SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "PlasmaRecordMe.h"
#include <QCoreApplication>
#include <QDir>
#include <QGuiApplication>
#include <QLoggingCategory>
#include <QProcess>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QQuickView>
#include <QRegularExpression>
#include <QScreen>
#include <QThread>
#include <QTimer>
#include <QUuid>


PlasmaRecordMe::PlasmaRecordMe(Screencasting::CursorMode cursorMode, const QStringList &sources, bool doSelection, QObject *parent)
    : QObject(parent)
    , m_cursorMode(cursorMode)
    , m_durationTimer(new QTimer(this))
    , m_sources(sources)
    , m_engine(new QQmlApplicationEngine(this))
{
    connect(m_engine, &QQmlEngine::quit, qGuiApp, &QCoreApplication::quit);
    m_engine->setInitialProperties({
        { QStringLiteral("app"), QVariant::fromValue<QObject *>(this) },
    });
    m_engine->load(QUrl(QStringLiteral("qrc:/main.qml")));

    m_screencasting = new Screencasting(this);

    m_durationTimer->setSingleShot(true);

    for (auto screen : qGuiApp->screens()) {
        addScreen(screen);
    }
    connect(qGuiApp, &QGuiApplication::screenAdded, this, &PlasmaRecordMe::addScreen);

    if (m_sources.isEmpty() || m_sources.contains(QLatin1String("region"))) {
        connect(this, &PlasmaRecordMe::workspaceChanged, this, [this] {
            qDeleteAll(m_workspaceStreams);
            m_workspaceStreams += m_screencasting->createRegionStream(m_workspace, 1, m_cursorMode);
            m_workspaceStreams += m_screencasting->createRegionStream(m_workspace, 1, m_cursorMode);
            start(m_workspaceStreams[0], true);
            start(m_workspaceStreams[1], false);
        });
    }

    if (doSelection) {
        requestSelection();
    }

    for (const auto &source : m_sources) {
        bool ok = false;
        auto node = source.toInt(&ok);
        if (ok) {
            startNode(node);
        }
    }

    for (auto screen : qGuiApp->screens()) {
        m_workspace |= screen->geometry();
    }
    Q_EMIT workspaceChanged();
}

PlasmaRecordMe::~PlasmaRecordMe()
{
}

bool PlasmaRecordMe::matches(const QString &value)
{
    if (m_sources.isEmpty()) {
        return true;
    }

    for (const auto &source : m_sources) {
        const QRegularExpression rx(source);
        if (rx.match(value).hasMatch()) {
            return true;
        }
    }
    return false;
}

void PlasmaRecordMe::addWindow(const QVariantList &uuid, const QString &appId)
{
    if (matches(appId) && uuid.size() > 0) {
        QString uuidString = uuid.at(0).value<QString>();
        auto stream = m_screencasting->createWindowStream(uuidString, m_cursorMode);
        stream->setObjectName(appId);
        start(stream, true);
        stream = m_screencasting->createWindowStream(uuidString, m_cursorMode);
        stream->setObjectName(appId);
        start(stream, false);
    }
}

void PlasmaRecordMe::addStream(int nodeid, const QString &displayText, int fd, bool allowDmaBuf)
{
    const auto roots = m_engine->rootObjects();
    for (auto root : roots) {
        auto mo = root->metaObject();
        mo->invokeMethod(root,
                         "addStream",
                         Q_ARG(QVariant, QVariant::fromValue<int>(nodeid)),
                         Q_ARG(QVariant, displayText),
                         Q_ARG(QVariant, fd),
                         Q_ARG(QVariant, allowDmaBuf));
    }
}

void PlasmaRecordMe::startNode(int node)
{
    addStream(node, QStringLiteral("raw node %1").arg(node), 0, true);
}

void PlasmaRecordMe::addScreen(QScreen *screen)
{
    auto f = [this, screen] {
        start(m_screencasting->createOutputStream(screen, m_cursorMode), false);
        start(m_screencasting->createOutputStream(screen, m_cursorMode), true);
    };
    if (matches(screen->name())) {
        connect(this, &PlasmaRecordMe::cursorModeChanged, screen, f);
        f();
    } else if (matches(screen->model())) {
        connect(this, &PlasmaRecordMe::cursorModeChanged, screen, f);
        f();
    }
}

void PlasmaRecordMe::start(ScreencastingStream *stream, bool allowDmaBuf)
{
    qDebug() << "start" << stream;
    connect(stream, &ScreencastingStream::failed, this, [this] (const QString &error) {
        qWarning() << "stream failed" << error;const auto roots = m_engine->rootObjects();
        for (auto root : roots) {
            auto mo = root->metaObject();
            mo->invokeMethod(root, "showPassiveNotification", Qt::QueuedConnection, Q_ARG(QVariant, QVariant(error))
                                                                                  , Q_ARG(QVariant, {})
                                                                                  , Q_ARG(QVariant, {})
                                                                                  , Q_ARG(QVariant, {})
            );
        }
    });
    connect(stream, &ScreencastingStream::closed, this, [this, stream] {
        auto nodeId = stream->property("nodeid").toInt();
        qDebug() << "bye bye" << stream << nodeId;

        const auto roots = m_engine->rootObjects();
        for (auto root : roots) {
            auto mo = root->metaObject();
            mo->invokeMethod(root, "removeStream", Qt::QueuedConnection, Q_ARG(QVariant, QVariant::fromValue<quint32>(nodeId)));
        }
    });
    connect(stream, &ScreencastingStream::created, this, [this, stream, allowDmaBuf](quint32 nodeId) {
        stream->setProperty("nodeid", nodeId);
        qDebug() << "starting..." << nodeId;
        addStream(nodeId, stream->objectName(), 0, allowDmaBuf);
    });
    connect(this, &PlasmaRecordMe::cursorModeChanged, stream, &ScreencastingStream::closed);
}

void PlasmaRecordMe::setDuration(int duration)
{
    m_durationTimer->setInterval(duration);
}

void PlasmaRecordMe::createVirtualMonitor()
{
    m_screencasting->createVirtualMonitorStream(QStringLiteral("recordme"), {1920, 1080}, 1, m_cursorMode);
}

void PlasmaRecordMe::setRegionPressed(const QString &screenName, int x, int y)
{
    const auto screens = QGuiApplication::screens();
    auto screenIt = std::find_if(screens.begin(), screens.end(), [&screenName](auto screen) {
        return screen->name() == screenName;
    });
    m_region.setTopLeft((*screenIt)->geometry().topLeft() + QPoint{x, y});
}

void PlasmaRecordMe::setRegionMoved(const QString &screenName, int x, int y)
{
    const auto screens = QGuiApplication::screens();
    auto screenIt = std::find_if(screens.begin(), screens.end(), [&screenName](auto screen) {
        return screen->name() == screenName;
    });
    m_region.setBottomRight((*screenIt)->geometry().topLeft() + QPoint{x, y});
    Q_EMIT regionChanged(m_region);
}

void PlasmaRecordMe::setRegionReleased(const QString &screenName, int x, int y)
{
    setRegionMoved(screenName, x, y);
    m_region = m_region.normalized();
    Q_EMIT regionFinal(m_region);
}

void PlasmaRecordMe::requestSelection()
{
    for (auto *screen : qApp->screens()) {
        auto view = new QQuickView(m_engine, nullptr);
        view->setScreen(screen);
        view->setInitialProperties({{QStringLiteral("app"), QVariant::fromValue<QObject *>(this)}});
        view->setSource(QUrl(QStringLiteral("qrc:/RegionSelector.qml")));
        view->setColor(Qt::transparent);
        view->showFullScreen();
        connect(this, &PlasmaRecordMe::regionFinal, view, &QQuickView::deleteLater);
    }

    connect(this, &PlasmaRecordMe::regionFinal, this, [this](const QRect &region) {
        for (auto stream : m_regionStreams) {
            Q_EMIT stream->closed();
        }
        qDeleteAll(m_regionStreams);
        m_regionStreams += m_screencasting->createRegionStream(region, 1, m_cursorMode);
        m_regionStreams += m_screencasting->createRegionStream(region, 1, m_cursorMode);
        start(m_regionStreams[0], true);
        start(m_regionStreams[1], false);
    });
}

QRect PlasmaRecordMe::region() const
{
    return m_region.normalized();
}

#include "moc_PlasmaRecordMe.cpp"
