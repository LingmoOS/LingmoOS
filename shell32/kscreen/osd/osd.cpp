/*
    SPDX-FileCopyrightText: 2014 Martin Klapetek <mklapetek@kde.org>
    SPDX-FileCopyrightText: 2016 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "osd.h"

#include <KScreen/Mode>

#include <LayerShellQt/Window>

#include <KWindowSystem>

#include <QCursor>
#include <QGuiApplication>
#include <QQuickItem>
#include <QScreen>
#include <QStandardPaths>
#include <QTimer>

#include <QQuickView>

using namespace KScreen;

Osd::Osd(const KScreen::OutputPtr &output, QObject *parent)
    : QObject(parent)
    , m_output(output)
{
    connect(output.data(), &KScreen::Output::isConnectedChanged, this, &Osd::onOutputAvailabilityChanged);
    connect(output.data(), &KScreen::Output::isEnabledChanged, this, &Osd::onOutputAvailabilityChanged);
}

Osd::~Osd()
{
}

void Osd::showActionSelector()
{
    if (!m_osdActionSelector) {
        m_osdActionSelector = std::make_unique<QQuickView>(&m_engine, nullptr);
        m_osdActionSelector->setInitialProperties({{QLatin1String("actions"), QVariant::fromValue(OsdAction::availableActions())}});
        m_osdActionSelector->setSource(QUrl(QStringLiteral("qrc:/qml/OsdSelector.qml")));
        m_osdActionSelector->setColor(Qt::transparent);
        m_osdActionSelector->setFlag(Qt::FramelessWindowHint);

        if (m_osdActionSelector->status() != QQuickView::Ready) {
            qWarning() << "Failed to load OSD QML file";
            m_osdActionSelector.reset();
            return;
        }

        auto rootObject = m_osdActionSelector->rootObject();
        connect(rootObject, SIGNAL(clicked(int)), this, SLOT(onOsdActionSelected(int)));
    }

    if (KWindowSystem::isPlatformWayland()) {
        auto layerWindow = LayerShellQt::Window::get(m_osdActionSelector.get());
        layerWindow->setScope(QStringLiteral("on-screen-display"));
        layerWindow->setLayer(LayerShellQt::Window::LayerOverlay);
        layerWindow->setAnchors({});
        layerWindow->setDesiredOutput(qGuiApp->screenAt(m_output->pos()));
    } else {
        auto newGeometry = m_osdActionSelector->geometry();
        auto screen = qGuiApp->screenAt(m_output->pos());
        newGeometry.moveCenter(screen ? screen->geometry().center() : qGuiApp->primaryScreen()->geometry().center());
        m_osdActionSelector->setGeometry(newGeometry);
        KWindowSystem::setState(m_osdActionSelector->winId(), NET::SkipPager | NET::SkipSwitcher | NET::SkipTaskbar);
        KWindowSystem::setType(m_osdActionSelector->winId(), NET::OnScreenDisplay);
        m_osdActionSelector->requestActivate();
    }
    m_osdActionSelector->setVisible(true);
}

void Osd::onOsdActionSelected(int action)
{
    Q_EMIT osdActionSelected(static_cast<OsdAction::Action>(action));
    hideOsd();
}

void Osd::onOutputAvailabilityChanged()
{
    if (!m_output || !m_output->isConnected() || !m_output->isEnabled() || !m_output->currentMode()) {
        hideOsd();
    }
}

void Osd::hideOsd()
{
    if (m_osdActionSelector) {
        m_osdActionSelector->setVisible(false);
    }
}
