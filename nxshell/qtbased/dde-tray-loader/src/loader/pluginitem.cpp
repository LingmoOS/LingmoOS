// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "constants.h"
#include "pluginitem.h"
#include "plugin.h"
#include "widgetplugin.h"

#include <QHBoxLayout>
#include <QMouseEvent>
#include <QMenu>

const static QString DockQuickPlugins = "Dock_Quick_Plugins";

PluginItem::PluginItem(PluginsItemInterface *pluginItemInterface, const QString &itemKey, QWidget *parent)
    : QWidget(parent)
    , m_pluginsItemInterface(pluginItemInterface)
    , m_pluginsItemInterfacev2(dynamic_cast<PluginsItemInterfaceV2*>(pluginItemInterface))
    , m_itemKey(itemKey)
    , m_menu(new QMenu(this))
    , m_tooltipTimer(new QTimer(this))
    , m_tipsWidget(nullptr)
{
    m_tooltipTimer->setSingleShot(true);
    m_tooltipTimer->setInterval(200);

    if (m_dbusProxy.isNull())
        m_dbusProxy.reset(new DockDBusProxy);

    connect(m_menu, &QMenu::triggered, this, [this](QAction *action){
        QString actionStr = action->data().toString();
        if (actionStr == Dock::dockMenuItemId || actionStr == Dock::unDockMenuItemId) {
            m_dbusProxy->setItemOnDock(DockQuickPlugins, m_itemKey, actionStr == Dock::dockMenuItemId);
        } else {
            m_pluginsItemInterface->invokedMenuItem(m_itemKey, action->data().toString(), action->isCheckable() ? action->isChecked() : true);
        }
    });
}

PluginItem::~PluginItem() = default;

QWidget *PluginItem::itemPopupApplet()
{
    if (auto popup = m_pluginsItemInterface->itemPopupApplet(m_itemKey)) {
        bool existed = panelPopupExisted();
        bool embed = embedPanelPopupExisted();

        if (existed) {
            Plugin::PluginPopup::remove(popup->windowHandle());
        }

        // hided, when clicked again
        if (existed && !embed) {
            return nullptr;
        }

        dock::WidgetPlugin::updateDockContainerState(m_pluginsItemInterface, true);

        popup->setParent(nullptr);
        popup->setAttribute(Qt::WA_TranslucentBackground);
        popup->winId();

        auto pluginPopup = Plugin::PluginPopup::get(popup->windowHandle());
        connect(pluginPopup, &Plugin::PluginPopup::eventGeometry, this, &PluginItem::updatePopupSize);

        pluginPopup->setPluginId(m_pluginsItemInterface->pluginName());
        pluginPopup->setItemKey(m_itemKey);
        pluginPopup->setPopupType(Plugin::PluginPopup::PopupTypePanel);
        return popup;
    }
    return nullptr;
}

QMenu *PluginItem::pluginContextMenu()
{
    if (!m_menu->actions().isEmpty()) {
        m_menu->clear();
    }

    initPluginMenu();
    if (m_pluginFlags & Dock::Attribute_CanSetting) {
        if (!m_unDockAction) {
            m_unDockAction = new QAction();
        }
        m_unDockAction->setEnabled(true);
        m_menu->addAction(m_unDockAction);

        m_unDockAction->setText(tr("Remove from dock"));
        m_unDockAction->setData(Dock::unDockMenuItemId);
    }

    qDebug() << "mouseRightButtonClicked:" << m_itemKey << m_menu->actions().size();

    if (m_menu->isEmpty())
        return nullptr;

    m_menu->setAttribute(Qt::WA_TranslucentBackground, true);
    // FIXME: qt5integration drawMenuItemBackground will draw a background event is's transparent
    auto pa = this->palette();
    pa.setColor(QPalette::ColorRole::Window, Qt::transparent);
    m_menu->setPalette(pa);
    m_menu->winId();

    auto geometry = windowHandle()->geometry();
    auto pluginPopup = Plugin::PluginPopup::get(m_menu->windowHandle());
    pluginPopup->setPluginId(m_pluginsItemInterface->pluginName());
    pluginPopup->setItemKey(m_itemKey);
    pluginPopup->setPopupType(Plugin::PluginPopup::PopupTypeMenu);
    m_menu->setFixedSize(m_menu->sizeHint());
    return m_menu;
}

void PluginItem::mousePressEvent(QMouseEvent *e)
{
    closeToolTip();
    QWidget::mousePressEvent(e);
}

void PluginItem::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        if (executeCommand())
            return;

        if (auto popup = itemPopupApplet()) {
            if (auto pluginPopup = Plugin::PluginPopup::get(popup->windowHandle())) {
                auto geometry = windowHandle()->geometry();
                const auto offset = QPoint(0, 0);
                pluginPopup->setX(geometry.x() + offset.x());
                pluginPopup->setY(geometry.y() + offset.y());
                popup->show();
            }
        }
    } else if (e->button() == Qt::RightButton) {
        if (auto menu = pluginContextMenu()) {
            if (auto pluginPopup = Plugin::PluginPopup::get(menu->windowHandle())) {
                auto geometry = windowHandle()->geometry();
                const auto offset = e->pos();
                pluginPopup->setX(geometry.x() + offset.x());
                pluginPopup->setY(geometry.y() + offset.y());
                menu->show();
            }
        }
    }
    QWidget::mouseReleaseEvent(e);
}

void PluginItem::enterEvent(QEvent *event)
{
    // panel popup existed, not show tooltip
    if (panelPopupExisted()) {
        return QWidget::enterEvent(event);
    }

    if (auto toolTip = pluginTooltip()) {
        if (auto pluginPopup = Plugin::PluginPopup::get(toolTip->windowHandle())) {
            auto geometry = windowHandle()->geometry();
            auto e = dynamic_cast<QEnterEvent *>(event);
            const auto offset = QPoint(width() / 2, height() / 2);
            pluginPopup->setX(geometry.x() + offset.x());
            pluginPopup->setY(geometry.y() + offset.y());

            connect(m_tooltipTimer, &QTimer::timeout, toolTip, [this, toolTip] {
                toolTip->show();
                m_tooltipTimer->disconnect(toolTip);
            });
            m_tooltipTimer->start();
        }
    }

    QWidget::enterEvent(event);
}

void PluginItem::moveEvent(QMoveEvent *e)
{
    if (auto popup = m_pluginsItemInterface->itemPopupApplet(m_itemKey); popup && popup->isVisible()) {
        if (auto pluginPopup = Plugin::PluginPopup::get(popup->windowHandle())) {
            auto geometry = windowHandle()->geometry();
            const auto offset = QPoint(0, 0);
            pluginPopup->setX(geometry.x() + offset.x());
            pluginPopup->setY(geometry.y() + offset.y());
        }
    }
    QWidget::moveEvent(e);
}

void PluginItem::leaveEvent(QEvent *event)
{
    closeToolTip();
}

QWidget* PluginItem::centralWidget()
{
    const int trayItemWidth = 16;
    const int trayItemHeight = 16;

    auto trayItemWidget = m_pluginsItemInterface->itemWidget(m_itemKey);
    auto policy = m_pluginsItemInterface->pluginSizePolicy();
    if (policy == PluginsItemInterface::System) {
        trayItemWidget->setFixedSize(trayItemWidth, trayItemHeight);
    } else {
        auto size = trayItemWidget->sizeHint();
        if (size.width() > 0 && size.height() > 0) {
            trayItemWidget->setFixedSize(trayItemWidget->sizeHint());
        }
    }

    return trayItemWidget;
}

PluginsItemInterface * PluginItem::pluginsItemInterface()
{
    return m_pluginsItemInterface;
}

void PluginItem::updateItemWidgetSize(const QSize &size)
{
    centralWidget()->setFixedSize(size);
    update();
}

int PluginItem::pluginFlags() const
{
    return m_pluginFlags;
}

void PluginItem::setPluginFlags(int flags)
{
    m_pluginFlags = flags;
}

void PluginItem::updatePopupSize(const QRect &rect)
{
    if (auto popup = m_pluginsItemInterface->itemPopupApplet(m_itemKey)) {
        popup->setFixedSize(rect.size());
        popup->update();
    }
}

void PluginItem::init()
{
    winId();
    setAttribute(Qt::WA_TranslucentBackground);

    auto hLayout = new QHBoxLayout;
    hLayout->addWidget(centralWidget());
    hLayout->setMargin(0);
    hLayout->setSpacing(0);
    hLayout->setSizeConstraint(QLayout::SetFixedSize);

    setLayout(hLayout);
}

void PluginItem::initPluginMenu()
{
    const QString menuJson = m_pluginsItemInterface->itemContextMenu(m_itemKey);
    if (menuJson.isEmpty()) {
        qWarning() << "itemContextMenu is empty!";
        return;
    }

    QJsonDocument jsonDocument = QJsonDocument::fromJson(menuJson.toLocal8Bit().data());
    if (jsonDocument.isNull()) {
        qWarning() << "jsonDocument is null!";
        return;
    }

    QJsonObject jsonMenu = jsonDocument.object();

    QJsonArray jsonMenuItems = jsonMenu.value("items").toArray();
    for (auto item : jsonMenuItems) {
        QJsonObject itemObj = item.toObject();
        QAction *action = new QAction(itemObj.value("itemText").toString());
        action->setCheckable(itemObj.value("isCheckable").toBool());
        action->setChecked(itemObj.value("checked").toBool());
        action->setData(itemObj.value("itemId").toString());
        action->setEnabled(itemObj.value("isActive").toBool());
        m_menu->addAction(action);
    }
}

QWidget *PluginItem::pluginTooltip()
{
    auto popup = m_pluginsItemInterface->itemPopupApplet(m_itemKey);
    if (popup && popup->isVisible())
        popup->windowHandle()->hide();

    return itemTooltip(m_itemKey);
}

QWidget * PluginItem::itemTooltip(const QString &itemKey)
{
    if (!m_tipsWidget) {
        auto toolTip = m_pluginsItemInterface->itemTipsWidget(itemKey);
        if (!toolTip) {
            qDebug() << "no tooltip";
            return nullptr;
        }
        m_tipsWidget = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout(m_tipsWidget);
        layout->setMargin(0);
        // add content margin, tooltip popup do not need to set padding
        layout->setContentsMargins(8, 4, 8, 4);
        layout->addWidget(toolTip);
        // fixme: 这个设置会导致widget resize, 当任务栏在右的时候，窗口变化会会遮住鼠标导致leaveEvent触发，tooltip消失
        if (itemKey.contains("network")) {
            layout->setSizeConstraint(QLayout::SetFixedSize);
        }
        toolTip->setVisible(true);
    } else {
        // can update tooltip content
        m_pluginsItemInterface->itemTipsWidget(itemKey);
    }

    m_tipsWidget->setParent(nullptr);
    m_tipsWidget->setAttribute(Qt::WA_TranslucentBackground);
    m_tipsWidget->winId();

    auto geometry = windowHandle()->geometry();
    auto pluginPopup = Plugin::PluginPopup::get(m_tipsWidget->windowHandle());
    pluginPopup->setPluginId(m_pluginsItemInterface->pluginName());
    pluginPopup->setItemKey(itemKey);
    pluginPopup->setPopupType(Plugin::PluginPopup::PopupTypeTooltip);
    return m_tipsWidget;
}

bool PluginItem::executeCommand()
{
    const QString command = m_pluginsItemInterface->itemCommand(m_itemKey);
    if (!command.isEmpty()) {
        qInfo() << "command: " << command;
        QStringList commandList = command.split(" ");
        QString program = commandList.takeFirst(); // 剩下是参数

        QProcess::startDetached(program, commandList);
        return true;
    }
    return false;
}

bool PluginItem::panelPopupExisted() const
{
    if (auto popup = m_pluginsItemInterface->itemPopupApplet(m_itemKey)) {
        if (Plugin::PluginPopup::contains(popup->windowHandle())) {
            return true;
        }
    }

    return false;
}

bool PluginItem::embedPanelPopupExisted() const
{
    if (auto popup = m_pluginsItemInterface->itemPopupApplet(m_itemKey)) {
        if(auto pluginPopup = Plugin::PluginPopup::getWithoutCreating(popup->windowHandle())) {
            return pluginPopup->popupType() == Plugin::PluginPopup::PopupTypeEmbed;
        }
    }
    return false;
}

void PluginItem::closeToolTip()
{
    if (m_tooltipTimer->isActive()) {
        m_tooltipTimer->stop();
    }

    if (m_tipsWidget && m_tipsWidget->windowHandle() && m_tipsWidget->windowHandle()->isVisible())
        m_tipsWidget->windowHandle()->hide();
}

void PluginItem::updatePluginContentMargin(int spacing)
{
    if (spacing > 0) {
        setContentsMargins(spacing, spacing, spacing, spacing);
        setFixedSize(sizeHint());
    }
}

bool PluginItem::isForceUndock(int pluginFlag)
{
    if (pluginFlag & Attribute_ForceUnDock) {
        return true;
    }

    return false;
}
