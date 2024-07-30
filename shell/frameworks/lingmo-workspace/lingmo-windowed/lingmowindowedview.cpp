/*
    SPDX-FileCopyrightText: 2014 Bhushan Shah <bhush94@gmail.com>
    SPDX-FileCopyrightText: 2014 Marco Martin <notmart@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "lingmowindowedview.h"

#include <QMenu>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlExpression>
#include <QQmlProperty>
#include <QQuickItem>
#include <QResizeEvent>

#include <LingmoQuick/AppletQuickItem>

#include <KActionCollection>
#include <KIconLoader>
#include <KLocalizedString>
#include <KStatusNotifierItem>

LingmoWindowedView::LingmoWindowedView(QWindow *parent)
    : QQuickView(parent)
    , m_applet(nullptr)
    , m_statusNotifier(nullptr)
    , m_withStatusNotifier(false)
{
    engine()->rootContext()->setContextProperty(QStringLiteral("root"), contentItem());
    // access appletInterface.Layout.minimumWidth, to create the Layout attached object for appletInterface as a sideeffect
    QQmlExpression *expr = new QQmlExpression(
        engine()->rootContext(),
        contentItem(),
        QStringLiteral("Qt.createQmlObject('import QtQuick; import QtQuick.Layouts; import org.kde.lingmoui as LingmoUI; "
                       "Rectangle {color: LingmoUI.Theme.backgroundColor; anchors.fill:parent; "
                       "property Item appletInterface; onAppletInterfaceChanged: print(appletInterface?.Layout.minimumWidth)}', root, \"\");"));
    m_rootObject = expr->evaluate().value<QQuickItem *>();
}

LingmoWindowedView::~LingmoWindowedView()
{
}

void LingmoWindowedView::setHasStatusNotifier(bool stay)
{
    Q_ASSERT(!m_statusNotifier);
    m_withStatusNotifier = stay;
}

void LingmoWindowedView::setApplet(Lingmo::Applet *applet)
{
    m_applet = applet;
    if (!applet) {
        return;
    }

    m_appletInterface = LingmoQuick::AppletQuickItem::itemForApplet(applet);

    if (!m_appletInterface) {
        return;
    }

    m_appletInterface->setParentItem(m_rootObject);
    m_rootObject->setProperty("appletInterface", QVariant::fromValue(m_appletInterface.data()));
    m_appletInterface->setVisible(true);
    setTitle(applet->title());
    setIcon(QIcon::fromTheme(applet->icon()));

    const QSize switchSize(m_appletInterface->property("switchWidth").toInt(), m_appletInterface->property("switchHeight").toInt());
    QRect geom = m_applet->config().readEntry("geometry", QRect());

    if (geom.isValid()) {
        geom.setWidth(qMax(geom.width(), switchSize.width() + 1));
        geom.setHeight(qMax(geom.height(), switchSize.height() + 1));
        setGeometry(geom);
    }
    setMinimumSize(QSize(qMax((int)KIconLoader::SizeEnormous, switchSize.width() + 1), qMax((int)KIconLoader::SizeEnormous, switchSize.height() + 1)));

    for (const auto children = m_appletInterface->children(); QObject * child : children) {
        // find for the needed property of Layout: minimum/maximum/preferred sizes and fillWidth/fillHeight
        if (child->property("minimumWidth").isValid() && child->property("minimumHeight").isValid() && child->property("preferredWidth").isValid()
            && child->property("preferredHeight").isValid() && child->property("maximumWidth").isValid() && child->property("maximumHeight").isValid()
            && child->property("fillWidth").isValid() && child->property("fillHeight").isValid()) {
            m_layout = child;
        }
    }

    if (m_layout) {
        connect(m_layout, SIGNAL(minimumWidthChanged()), this, SLOT(minimumWidthChanged()));
        connect(m_layout, SIGNAL(minimumHeightChanged()), this, SLOT(minimumHeightChanged()));
    }
    minimumWidthChanged();
    minimumHeightChanged();
    QObject::connect(applet->containment(),
                     &Lingmo::Containment::configureRequested,
                     this,
                     static_cast<void (LingmoWindowedView::*)(Lingmo::Applet *)>(&LingmoWindowedView::showConfigurationInterface));

    Q_ASSERT(!m_statusNotifier);
    if (m_withStatusNotifier) {
        m_statusNotifier = new KStatusNotifierItem(applet->pluginMetaData().pluginId(), this);
        m_statusNotifier->setStandardActionsEnabled(false); // we add our own "Close" entry manually below

        updateSniIcon();
        connect(applet, &Lingmo::Applet::iconChanged, this, &LingmoWindowedView::updateSniIcon);

        updateSniTitle();
        connect(applet, &Lingmo::Applet::titleChanged, this, &LingmoWindowedView::updateSniTitle);

        updateSniStatus();
        connect(applet, &Lingmo::Applet::statusChanged, this, &LingmoWindowedView::updateSniStatus);

        // set up actions
        for (auto a : applet->contextualActions()) {
            m_statusNotifier->contextMenu()->addAction(a);
        }
        m_statusNotifier->contextMenu()->addSeparator();
        QAction *closeAction = new QAction(QIcon::fromTheme(QStringLiteral("window-close")), i18n("Close %1", applet->title()), this);
        connect(closeAction, &QAction::triggered, this, [this]() {
            m_statusNotifier->deleteLater();
            close();
        });
        m_statusNotifier->contextMenu()->addAction(closeAction);

        connect(m_statusNotifier.data(), &KStatusNotifierItem::activateRequested, this, [this](bool /*active*/, const QPoint & /*pos*/) {
            if (isVisible() && isActive()) {
                hide();
            } else {
                show();
                raise();
            }
        });
        auto syncStatus = [this]() {
            switch (m_applet->status()) {
            case Lingmo::Types::AcceptingInputStatus:
            case Lingmo::Types::RequiresAttentionStatus:
            case Lingmo::Types::NeedsAttentionStatus:
                m_statusNotifier->setStatus(KStatusNotifierItem::NeedsAttention);
                break;
            case Lingmo::Types::ActiveStatus:
                m_statusNotifier->setStatus(KStatusNotifierItem::Active);
                break;
            default:
                m_statusNotifier->setStatus(KStatusNotifierItem::Passive);
            }
        };
        connect(applet, &Lingmo::Applet::statusChanged, this, syncStatus);
        syncStatus();
    }
}

void LingmoWindowedView::showConfigurationInterface()
{
    if (!m_applet) {
        return;
    }

    if (!m_configView) {
        m_configView = new LingmoQuick::ConfigView(m_applet);
        m_configView->init();
        m_configView->setTransientParent(this);
    }

    m_configView->show();
    m_configView->requestActivate();
}

void LingmoWindowedView::resizeEvent(QResizeEvent *ev)
{
    if (!m_applet) {
        return;
    }

    QQuickItem *i = LingmoQuick::AppletQuickItem::itemForApplet(m_applet);
    if (!i) {
        return;
    }

    minimumWidthChanged();
    minimumHeightChanged();

    i->setSize(ev->size());
    contentItem()->setSize(ev->size());

    m_applet->config().writeEntry("geometry", QRect(position(), ev->size()));
}

void LingmoWindowedView::mouseReleaseEvent(QMouseEvent *ev)
{
    QQuickWindow::mouseReleaseEvent(ev);

    if ((!(ev->buttons() & Qt::RightButton) && ev->button() != Qt::RightButton) || ev->isAccepted()) {
        return;
    }

    Q_EMIT m_applet->contextualActionsAboutToShow();

    QMenu menu;

    for (const auto actions = m_applet->contextualActions(); QAction * action : actions) {
        if (action) {
            menu.addAction(action);
        }
    }

    if (!m_applet->failedToLaunch()) {
        QAction *configureApplet = m_applet->internalAction(QStringLiteral("configure"));
        if (configureApplet && configureApplet->isEnabled()) {
            menu.addAction(configureApplet);
        }
    }

    menu.exec(ev->globalPosition().toPoint());
    ev->setAccepted(true);
}

void LingmoWindowedView::keyPressEvent(QKeyEvent *ev)
{
    if (ev->matches(QKeySequence::Quit)) {
        m_statusNotifier->deleteLater();
        close();
    }
    QQuickView::keyReleaseEvent(ev);
}

void LingmoWindowedView::moveEvent(QMoveEvent *ev)
{
    Q_UNUSED(ev)
    m_applet->config().writeEntry("geometry", QRect(position(), size()));
}

void LingmoWindowedView::hideEvent(QHideEvent *ev)
{
    Q_UNUSED(ev)
    m_applet->config().sync();
    if (!m_withStatusNotifier) {
        m_applet->deleteLater();
        deleteLater();
    }
}

void LingmoWindowedView::showConfigurationInterface(Lingmo::Applet *applet)
{
    if (m_configView) {
        m_configView->hide();
        m_configView->deleteLater();
    }

    if (!applet || !applet->containment()) {
        return;
    }

    m_configView = new LingmoQuick::ConfigView(applet);

    m_configView->init();
    m_configView->show();
}

void LingmoWindowedView::minimumWidthChanged()
{
    if (!m_layout || !m_appletInterface) {
        return;
    }

    setMinimumWidth(qMax(m_appletInterface->property("switchWidth").toInt() + 1, m_layout->property("minimumWidth").toInt()));
}

void LingmoWindowedView::minimumHeightChanged()
{
    if (!m_layout || !m_appletInterface) {
        return;
    }

    setMinimumHeight(qMax(m_appletInterface->property("switchHeight").toInt() + 1, m_layout->property("minimumHeight").toInt()));
}

void LingmoWindowedView::maximumWidthChanged()
{
    if (!m_layout) {
        return;
    }

    setMaximumWidth(m_layout->property("maximumWidth").toInt());
}

void LingmoWindowedView::maximumHeightChanged()
{
    if (!m_layout) {
        return;
    }

    setMaximumHeight(m_layout->property("maximumHeight").toInt());
}

void LingmoWindowedView::updateSniIcon()
{
    m_statusNotifier->setIconByName(m_applet->icon());
}

void LingmoWindowedView::updateSniTitle()
{
    m_statusNotifier->setTitle(m_applet->title());
    m_statusNotifier->setToolTipTitle(m_applet->title());
}

void LingmoWindowedView::updateSniStatus()
{
    switch (m_applet->status()) {
    case Lingmo::Types::UnknownStatus:
    case Lingmo::Types::PassiveStatus:
    case Lingmo::Types::HiddenStatus:
        m_statusNotifier->setStatus(KStatusNotifierItem::Passive);
        break;
    case Lingmo::Types::ActiveStatus:
    case Lingmo::Types::AcceptingInputStatus:
        m_statusNotifier->setStatus(KStatusNotifierItem::Active);
        break;
    case Lingmo::Types::NeedsAttentionStatus:
    case Lingmo::Types::RequiresAttentionStatus:
        m_statusNotifier->setStatus(KStatusNotifierItem::NeedsAttention);
        break;
    }
}
