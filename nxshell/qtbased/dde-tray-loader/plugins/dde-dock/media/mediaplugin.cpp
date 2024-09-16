// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mediaplugin.h"
#include "mediacontroller.h"
#include "quickpanelwidget.h"
#include "plugins-logging-category.h"

#define MEDIA_KEY "media-key"
#define STATE_KEY  "enable"

Q_LOGGING_CATEGORY(MEDIA, "org.deepin.dde.dock.media")

MediaPlugin::MediaPlugin(QObject *parent)
    : QObject(parent)
    , m_quickPanelWidget(nullptr)
{

}

void MediaPlugin::init(PluginProxyInterface *proxyInter)
{
    m_proxyInter = proxyInter;

    m_controller.reset(new MediaController);
    m_quickPanelWidget.reset(new QuickPanelWidget(m_controller.data()));
    m_quickPanelWidget->setFixedHeight(60);

    if (pluginIsDisable()) {
        qCDebug(MEDIA) << "Media plugin init, plugin is disabled";
        return;
    }
    connect(m_controller.data(), &MediaController::mediaAcquired, this, [this] {
        m_proxyInter->itemAdded(this, MEDIA_KEY);
    });
    connect(m_controller.data(), &MediaController::mediaLosted, this, [this] {
        m_proxyInter->itemRemoved(this, MEDIA_KEY);
    });
    connect(m_quickPanelWidget.data(), &QuickPanelWidget::requestHideApplet, this, [this] {
        if (m_proxyInter)
            m_proxyInter->requestSetAppletVisible(this, MEDIA_KEY, false);
    });

    if (m_controller->isWorking())
        m_proxyInter->itemAdded(this, MEDIA_KEY);
}

const QString MediaPlugin::pluginName() const
{
    return "media";
}

const QString MediaPlugin::pluginDisplayName() const
{
    return "Media";
}

void MediaPlugin::pluginStateSwitched()
{
    m_proxyInter->saveValue(this, STATE_KEY, pluginIsDisable());
}

bool MediaPlugin::pluginIsDisable()
{
    return !m_proxyInter->getValue(this, STATE_KEY, true).toBool();
}

QWidget *MediaPlugin::itemWidget(const QString &itemKey)
{
    if (itemKey == Dock::QUICK_ITEM_KEY) {
        return m_quickPanelWidget.data();
    }

    return nullptr;
}

QWidget *MediaPlugin::itemTipsWidget(const QString &itemKey)
{
    return nullptr;
}

QWidget *MediaPlugin::itemPopupApplet(const QString &itemKey)
{
    return nullptr;
}

void MediaPlugin::refreshIcon(const QString &itemKey)
{

}

const QString MediaPlugin::itemContextMenu(const QString &itemKey)
{
    return QString();
}

void MediaPlugin::invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked)
{

}

int MediaPlugin::itemSortKey(const QString &itemKey)
{
    const QString key = QString("pos_%1_%2").arg(itemKey).arg(Dock::Efficient);

    return m_proxyInter->getValue(this, key, -1).toInt();
}

void MediaPlugin::setSortKey(const QString &itemKey, const int order)
{
    const QString key = QString("pos_%1_%2").arg(itemKey).arg(Dock::Efficient);

    m_proxyInter->saveValue(this, key, order);
}

void MediaPlugin::pluginSettingsChanged()
{
    refreshPluginItemsVisible();
}

void MediaPlugin::refreshPluginItemsVisible()
{
    if (pluginIsDisable())
        m_proxyInter->itemRemoved(this, MEDIA_KEY);
    else
        m_proxyInter->itemAdded(this, MEDIA_KEY);
}


