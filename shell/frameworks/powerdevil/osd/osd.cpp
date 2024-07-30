/*
    SPDX-FileCopyrightText: 2014 Martin Klapetek <mklapetek@kde.org>
    SPDX-FileCopyrightText: 2016 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2023 Natalie Clarius <natalie.clarius@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "osd.h"

#include <LayerShellQt/Window>

#include <KWindowSystem>
#include <KX11Extras>

#include <QCursor>
#include <QGuiApplication>
#include <QQuickItem>
#include <QScreen>
#include <QStandardPaths>
#include <QTimer>

#include "osdaction.h"

using namespace PowerDevil;

void Osd::showActionSelector(const QString &currentProfile)
{
    m_engine.setProperty("_kirigamiTheme", QStringLiteral("KirigamiPlasmaStyle"));

    if (!m_osdActionSelector) {
        m_osdActionSelector = std::make_unique<QQuickView>(&m_engine, nullptr);
        m_osdActionSelector->setInitialProperties({{QLatin1String("actions"), QVariant::fromValue(OsdAction::availableActions())}, {QLatin1String("currentProfile"), currentProfile}});
        m_osdActionSelector->setSource(QUrl(QStringLiteral("qrc:/qml/OsdSelector.qml")));
        m_osdActionSelector->setColor(Qt::transparent);
        m_osdActionSelector->setFlag(Qt::FramelessWindowHint);

        if (m_osdActionSelector->status() != QQuickView::Ready) {
            qWarning() << "Failed to load OSD QML file";
            m_osdActionSelector.reset();
            return;
        }

        connect(m_osdActionSelector.get(), &QQuickView::activeChanged, this, [this]() {
            Q_EMIT osdActiveChanged(m_osdActionSelector->isActive());
        });

        auto rootObject = m_osdActionSelector->rootObject();
        connect(rootObject, SIGNAL(clicked(QString)), this, SLOT(onOsdActionSelected(QString)));
    }

    auto screen = qGuiApp->primaryScreen();

    if (KWindowSystem::isPlatformWayland()) {
        auto layerWindow = LayerShellQt::Window::get(m_osdActionSelector.get());
        layerWindow->setScope(QStringLiteral("on-screen-display"));
        layerWindow->setLayer(LayerShellQt::Window::LayerOverlay);
        layerWindow->setAnchors({});
        layerWindow->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityOnDemand);
        m_osdActionSelector->setScreen(screen);
    } else {
        auto newGeometry = m_osdActionSelector->geometry();
        newGeometry.moveCenter(screen->geometry().center());
        m_osdActionSelector->setGeometry(newGeometry);
        KX11Extras::setState(m_osdActionSelector->winId(), NET::SkipPager | NET::SkipSwitcher | NET::SkipTaskbar);
        KX11Extras::setType(m_osdActionSelector->winId(), NET::OnScreenDisplay);
        m_osdActionSelector->requestActivate();
    }
    m_osdActionSelector->setVisible(true);
}

void Osd::onOsdActionSelected(const QString &action)
{
    Q_EMIT osdActionSelected(action);
    hideOsd();
}

void Osd::hideOsd()
{
    if (m_osdActionSelector) {
        m_osdActionSelector->setVisible(false);
    }
}

#include "moc_osd.cpp"
