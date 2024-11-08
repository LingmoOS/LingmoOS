/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: hxf <hewenfei@kylinos.cn>
 *          iaom <zhangpengfei@kylinos.cn>
 */

#include "panel.h"
#include <QUrl>
#include <windowmanager/windowmanager.h>
#include <KWindowSystem>
#include <QMapIterator>
#include <KWindowEffects>
#include <utility>
#include <QMenu>
#include <QGuiApplication>
#include <QGSettings>

#include <widget.h>
#include <widget-item.h>
#include <config-loader.h>
#include <app-launcher.h>
#include "general-config-define.h"
#include "widget-model.h"
#include "shell.h"
#include "data-collector.h"

namespace LingmoUIPanel {
#define DEFAULT_PANEL_SIZE 48
// 如果需要不同Panel加载不同的Config,将id设置为不同即可
Panel::Panel(Screen *screen, const QString &id, QWindow *parent)
  : LingmoUIQuick::IslandView(QStringLiteral("panel"), QStringLiteral("lingmo-panel")),
    m_id(id)
{
    qRegisterMetaType<QList<int>>();
    qmlRegisterType<LingmoUIPanel::WidgetSortModel>("org.lingmo.panel.impl", 1, 0, "WidgetSortModel");

    rootContext()->setContextProperty("panel", this);
    initIsland();

    setColor(Qt::transparent);
    setFlags(flags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus);

    m_windowProxy = new LingmoUIQuick::WindowProxy(this);
    m_windowProxy->setWindowType(LingmoUIQuick::WindowType::Dock);

    // contents
    setPanelScreen(screen);
    initConfig();
    loadWidgetsConfig();
    loadActions();

    setShowSystemTrayStatus();
    loadMainViewItem();

    // 加载任务栏设置
    initPanelConfig();

    // 用于执行隐藏的定时器
    m_hideTimer = new QTimer(this);
    m_hideTimer->setSingleShot(true);
    m_hideTimer->setInterval(500);
    connect(m_hideTimer, &QTimer::timeout, this, [this] {
        setHidden(true);
    });

    connect(mainView()->config(), &LingmoUIQuick::Config::configChanged, this, [this] (const QString &key) {
        if (key == QStringLiteral("disabledWidgets")) {
            QStringList disabledWidgets = mainView()->config()->getValue(key).toStringList();
            if (disabledWidgets == m_disabledWidgets) {
                return;
            }

            QStringList tmp;
            tmp.append(disabledWidgets);
            tmp.append(m_disabledWidgets);
            tmp.removeDuplicates();

            for (const auto &item : tmp) {
                disableWidget(item, disabledWidgets.contains(item));
            }
        }
    });
}

void Panel::initIsland()
{
    const QString defaultViewId = QStringLiteral("org.lingmo.panel");
    // 添加panel包所在的qrc路径
    LingmoUIQuick::WidgetContainer::widgetLoader().addWidgetSearchPath(QStringLiteral(":/lingmo-panel"));

    // TODO: 配置错误检查
    if (!loadMainViewWithoutItem(defaultViewId)) {
        // 全部使用默认配置
        auto metadata = LingmoUIQuick::WidgetMetadata(QStringLiteral(":/lingmo-panel/org.lingmo.panel"));
        auto cont = new LingmoUIQuick::WidgetContainer(metadata, this);
        cont->setConfig(LingmoUIQuick::ConfigLoader::getConfig(QStringLiteral("panel")));
        setMainView(cont);
    }
}

void Panel::setPanelScreen(Screen *screen)
{
    if (!screen || screen == m_screen) {
        return;
    }

    if (m_screen) {
        m_screen->disconnect(this);
    }

    m_screen = screen;
    setScreen(screen->internal());
    mainView()->setScreen(screen->internal());
    updateGeometry();
    connect(screen, &Screen::geometryChanged, this, &Panel::onScreenGeometryChanged);
}

void Panel::onScreenGeometryChanged(const QRect &geometry)
{
    Q_UNUSED(geometry)
    updateGeometry();
}

void Panel::initPanelConfig()
{
    auto config = mainView()->config();
    QVariant value = config->getValue(QStringLiteral("lockPanel"));
    if (!value.isValid()) {
        value = false;
        config->setValue(QStringLiteral("lockPanel"), value);
    }
    setLockPanel(value.toBool(), true);

    value = config->getValue(QStringLiteral("panelAutoHide"));
    if (!value.isValid()) {
        value = false;
        config->setValue(QStringLiteral("panelAutoHide"), value);
    }
    // 初始化为不自动隐藏
    setAutoHide(false);
    setAutoHide(value.toBool());

    connect(config, &LingmoUIQuick::Config::configChanged, this, [this, config] (const QString &key) {
        if (key == QStringLiteral("position")) {
            setPosition(LingmoUIQuick::Types::Pos(config->getValue(key).toInt()));
        } else if (key == QStringLiteral("customPanelSize")) {
            auto panelSize = config->getValue(key).value<int>();
            setPanelSize(panelSize);
        } else if (key == QStringLiteral("panelSizePolicy")) {
            auto policy = config->getValue(key).value<GeneralConfigDefine::PanelSizePolicy>();
            setPanelPolicy(policy);
        } else if (key == QStringLiteral("panelAutoHide")) {
            setAutoHide(config->getValue(key).toBool());
        } else if (key == QStringLiteral("lockPanel")) {
            setLockPanel(config->getValue(key).toBool());
        } else if (key == QStringLiteral("showSystemTrayOnAllPanels")) {
            //根据托盘是否显示在任务栏上面的值更新托盘的启用状态
            if(config->getValue(key).toBool()) {
                mainView()->addWidget(QStringLiteral("org.lingmo.systemTray"), m_systemTrayInstance);
            } else {
                if(m_screen->internal() != qApp->primaryScreen()) {
                    mainView()->removeWidget(QStringLiteral("org.lingmo.systemTray"));
                }
            }
        }
    });
}

void Panel::initConfig()
{
    // border, margin
    QMap<QString, int> defaultList;
    defaultList.insert(QStringLiteral("leftMargin"), 0);
    defaultList.insert(QStringLiteral("topMargin"), 0);
    defaultList.insert(QStringLiteral("rightMargin"), 0);
    defaultList.insert(QStringLiteral("bottomMargin"), 0);
    defaultList.insert(QStringLiteral("leftPadding"), 2);
    defaultList.insert(QStringLiteral("topPadding"), 2);
    defaultList.insert(QStringLiteral("rightPadding"), 2);
    defaultList.insert(QStringLiteral("bottomPadding"), 2);
    defaultList.insert(QStringLiteral("panelMinimumSize"), 48);
    defaultList.insert(QStringLiteral("panelMaximumSize"), 92);
    defaultList.insert(QStringLiteral("panelSize"), DEFAULT_PANEL_SIZE);
    defaultList.insert(QStringLiteral("radius"), 0);
    defaultList.insert(QStringLiteral("position"), LingmoUIQuick::Types::Pos::Bottom);
    defaultList.insert(QStringLiteral("panelSizePolicy"), GeneralConfigDefine::Small);
    defaultList.insert(QStringLiteral("lockPanel"), false);
    defaultList.insert(QStringLiteral("panelAutoHide"), false);
    defaultList.insert(QStringLiteral("customPanelSize"), DEFAULT_PANEL_SIZE);

    QString version = config()->getValue(QStringLiteral("panelConfigVersion")).toString();
    auto viewConfig = mainView()->config();
    if(PANEL_CONFIG_VERSION != version) {
        for (const auto &key : defaultList.keys()) {
            if(!config()->getValue(key).isNull()) {
                defaultList.insert(key, config()->getValue(key).toInt());
                config()->removeKey(key);
            }
            if(viewConfig->getValue(key).isNull()) {
                viewConfig->setValue(key, defaultList.value(key));
            }
        }
        config()->setValue(QStringLiteral("panelConfigVersion"), PANEL_CONFIG_VERSION);
    }
    for (const auto &key : defaultList.keys()) {
        if(viewConfig->getValue(key).isNull()) {
            viewConfig->setValue(key, defaultList.value(key));
        }
    }

    // panelSize
    m_panelMaxSize = viewConfig->getValue(QStringLiteral("panelMaximumSize")).isNull()? 92 : viewConfig->getValue(QStringLiteral("panelMaximumSize")).toInt();
    m_panelMinSize = viewConfig->getValue(QStringLiteral("panelMinimumSize")).isNull()? 48 : viewConfig->getValue(QStringLiteral("panelMinimumSize")).toInt();

    int policy = viewConfig->getValue(QStringLiteral("panelSizePolicy")).isNull()? GeneralConfigDefine::Medium : viewConfig->getValue(QStringLiteral("panelSizePolicy")).toInt();
    if(policy == GeneralConfigDefine::Custom) {
        m_panelSize = viewConfig->getValue(QStringLiteral("customPanelSize")).isNull()? DEFAULT_PANEL_SIZE : viewConfig->getValue(QStringLiteral("customPanelSize")).toInt();
    } else {
        m_panelSize = viewConfig->getValue(QStringLiteral("panelSize")).isNull()? DEFAULT_PANEL_SIZE : viewConfig->getValue(QStringLiteral("panelSize")).toInt();
    }
    setPanelPolicy(static_cast<GeneralConfigDefine::PanelSizePolicy>(policy));

    // container
    auto cont = mainView();
    cont->setHost(LingmoUIQuick::WidgetMetadata::Host::Panel);
    cont->setRadius(viewConfig->getValue(QStringLiteral("radius")).toInt());

    auto margin = cont->margin();
    auto padding = cont->padding();
    margin->setLeft(viewConfig->getValue(QStringLiteral("leftMargin")).toInt());
    margin->setTop(viewConfig->getValue(QStringLiteral("topMargin")).toInt());
    margin->setRight(viewConfig->getValue(QStringLiteral("rightMargin")).toInt());
    margin->setBottom(viewConfig->getValue(QStringLiteral("bottomMargin")).toInt());
    padding->setLeft(viewConfig->getValue(QStringLiteral("leftPadding")).toInt());
    padding->setTop(viewConfig->getValue(QStringLiteral("topPadding")).toInt());
    padding->setRight(viewConfig->getValue(QStringLiteral("rightPadding")).toInt());
    padding->setBottom(viewConfig->getValue(QStringLiteral("bottomPadding")).toInt());

    // position
    int position = viewConfig->getValue(QStringLiteral("position")).isNull()? LingmoUIQuick::Types::Pos::Bottom : viewConfig->getValue(QStringLiteral("position")).toInt();
    setPosition(static_cast<LingmoUIQuick::Types::Pos>(position));
    connect(cont, &LingmoUIQuick::WidgetContainer::activeChanged, this, &Panel::onContainerActiveChanged);
}

void Panel::setPosition(LingmoUIQuick::Types::Pos position)
{
    auto container = Panel::mainView();
    if (position != container->position()) {
        // 任务栏位置与屏幕：上: 1, 下: 0, 左: 2, 右: 3, 如果为其他值，则说明任务栏不存在
        int panelPosition = 0;
        switch (position) {
            case LingmoUIQuick::Types::Left:
                panelPosition = 2;
                break;
            case LingmoUIQuick::Types::Top:
                panelPosition = 1;
                break;
            case LingmoUIQuick::Types::Right:
                panelPosition = 3;
                break;
            default:
            case LingmoUIQuick::Types::Bottom:
                panelPosition = 0;
                position = LingmoUIQuick::Types::Pos::Bottom;
                break;
        }

        container->setPosition(position);
        updateGeometry();

        if (position == LingmoUIQuick::Types::Left || position == LingmoUIQuick::Types::Right) {
            container->setOrientation(LingmoUIQuick::Types::Vertical);
        } else {
            container->setOrientation(LingmoUIQuick::Types::Horizontal);
        }

        mainView()->config()->setValue(QStringLiteral("position"), position);
        // 仅同步到gsetting
        QGSettings settings("org.lingmo.panel.settings", "/org/lingmo/panel/settings/", this);
        settings.set(QStringLiteral("panelposition"), panelPosition);
        //上传埋点数据
        DataCollector::setPanelPositionEvent(panelPosition);
    }

    if (m_positionAction) {
        updatePositionAction();
    }
}

void Panel::setPanelSize(int size)
{
    if (size == m_panelSize) {
        return;
    }

    if (size < m_panelMinSize) {
        size = m_panelMinSize;
    } else if (size > m_panelMaxSize) {
        size = m_panelMaxSize;
    }

    m_panelSize = size;
    updateGeometry();
    if(m_sizePolicy == GeneralConfigDefine::Custom) {
        mainView()->config()->setValue(QStringLiteral("customPanelSize"), m_panelSize);
    } else {
        mainView()->config()->setValue(QStringLiteral("panelSize"), m_panelSize);
    }

    // 仅同步到gsetting
    QGSettings settings("org.lingmo.panel.settings", "/org/lingmo/panel/settings/", this);
    // TODO: 修改任务栏尺寸m_panelSize为实际占用大小
    if (mainView()->orientation() == LingmoUIQuick::Types::Horizontal) {
        settings.set(QStringLiteral("panelsize"), m_panelSize + mainView()->margin()->top() + mainView()->margin()->bottom());
    } else {
        settings.set(QStringLiteral("panelsize"), m_panelSize + mainView()->margin()->right() + mainView()->margin()->left());
    }
    //上传埋点数据
    DataCollector::setPanelSizeEvent(size);
}

void Panel::loadWidgetsConfig()
{
    auto containerConfig = mainView()->config();
    containerConfig->addGroupInfo(QStringLiteral("widgets"), QStringLiteral("instanceId"));
    QStringList defaultWidgets {
        QStringLiteral("org.lingmo.menu.starter"),
        QStringLiteral("org.lingmo.panel.search"),
        QStringLiteral("org.lingmo.panel.taskView"),
        QStringLiteral("org.lingmo.panel.separator"),
        QStringLiteral("org.lingmo.panel.taskManager"),
        QStringLiteral("org.lingmo.systemTray"),
        QStringLiteral("org.lingmo.panel.calendar"),
        QStringLiteral("org.lingmo.panel.showDesktop")
    };
    if (containerConfig->numberOfChildren(QStringLiteral("widgets")) == 0) {
        // 默认加载widgets
        QVariantList order;
        for (const auto &widget : defaultWidgets) {
            int instanceId = order.count();
            order << instanceId;

            QVariantMap wData;
            wData.insert(QStringLiteral("id"), widget);
            wData.insert(QStringLiteral("instanceId"), instanceId);

            containerConfig->addChild(QStringLiteral("widgets"), wData);
            if(widget == QStringLiteral("org.lingmo.systemTray")) {
                m_systemTrayInstance = instanceId;
            }
            mainView()->addWidget(widget, instanceId);
        }

        containerConfig->setValue(QStringLiteral("widgetsOrder"), order);
        containerConfig->setValue(QStringLiteral("disabledWidgets"), QStringList());
    }

    m_disabledWidgets = containerConfig->getValue(QStringLiteral("disabledWidgets")).toStringList();
    // 卸载插件选项
    QStringList canRemove;
    canRemove << QStringLiteral("org.lingmo.panel.search");
    canRemove << QStringLiteral("org.lingmo.panel.taskView");

    defaultWidgets.removeDuplicates();
    for (const auto &wid : defaultWidgets) {
        if (canRemove.contains(wid)) {
            LingmoUIQuick::WidgetMetadata metadata = LingmoUIQuick::WidgetContainer::widgetLoader().loadMetadata(wid);
            if (!metadata.isValid()) {
                continue;
            }
            QString actionName = QString(tr("Show")) + metadata.name();
            auto action = new QAction(actionName);
            action->setProperty("widget", wid);
            action->setCheckable(true);
            action->setChecked(!m_disabledWidgets.contains(wid));

            m_widgetActions << action;
            connect(action, &QAction::triggered, this, [this, action] {
                widgetActionTriggered(action);
            });
        }
    }
}

void Panel::updateGeometry()
{
    refreshRect();
    updateMask();
    resetWorkArea(!m_autoHide);
}

void Panel::refreshRect()
{
    int panelSize = m_panelSize;
    auto container = Panel::mainView();
    const auto margin = container->margin();

    // panelRect 为实际占用的区域，包括外边距（margin）
    QRect screenGeometry = container->screen()->geometry(), panelRect;
    UnavailableArea unavailableArea;

    switch (container->position()) {
        default:
        case LingmoUIQuick::Types::Pos::Bottom:
            panelSize += (margin->top() + margin->bottom());
            panelRect = screenGeometry.adjusted(0, screenGeometry.height() - panelSize, 0, 0);

            unavailableArea.bottomWidth = panelRect.height();
            unavailableArea.bottomStart = panelRect.left();
            unavailableArea.bottomEnd   = panelRect.right();

            m_windowProxy->slideWindow(LingmoUIQuick::WindowProxy::BottomEdge);
            break;
        case LingmoUIQuick::Types::Pos::Left:
            panelSize += (margin->left() + margin->right());
            panelRect = screenGeometry.adjusted(0, 0, panelSize - screenGeometry.width(), 0);

            unavailableArea.leftWidth = panelRect.width();
            unavailableArea.leftStart = panelRect.top();
            unavailableArea.leftEnd   = panelRect.bottom();

            m_windowProxy->slideWindow(LingmoUIQuick::WindowProxy::LeftEdge);
            break;
        case LingmoUIQuick::Types::Pos::Top:
            panelSize += (margin->top() + margin->bottom());
            panelRect = screenGeometry.adjusted(0, 0, 0, panelSize - screenGeometry.height());

            unavailableArea.topWidth = panelRect.height();
            unavailableArea.topStart = panelRect.left();
            unavailableArea.topEnd   = panelRect.right();

            m_windowProxy->slideWindow(LingmoUIQuick::WindowProxy::TopEdge);
            break;
        case LingmoUIQuick::Types::Pos::Right:
            panelSize += (margin->left() + margin->right());
            panelRect = screenGeometry.adjusted(screenGeometry.width() - panelSize, 0, 0, 0);

            unavailableArea.rightWidth = panelRect.width();
            unavailableArea.rightStart = panelRect.top();
            unavailableArea.rightEnd   = panelRect.bottom();

            m_windowProxy->slideWindow(LingmoUIQuick::WindowProxy::RightEdge);
            break;
    }

    QRect rect = panelRect.adjusted(margin->left(), margin->top(), -margin->right(), -margin->bottom());
    container->setGeometry(rect);

    // TODO: 使用WindowProxy2
    m_windowProxy->setGeometry(panelRect);

    m_unavailableArea = unavailableArea;
}

void Panel::setPanelPolicy(GeneralConfigDefine::PanelSizePolicy sizePolicy)
{
    if (sizePolicy != m_sizePolicy) {
        switch (sizePolicy) {
            default:
            case GeneralConfigDefine::Small:
                m_sizePolicy = GeneralConfigDefine::Small;
                setPanelSize(48);
                break;
            case GeneralConfigDefine::Medium:
                m_sizePolicy = GeneralConfigDefine::Medium;
                setPanelSize(72);
                break;
            case GeneralConfigDefine::Large:
                m_sizePolicy = GeneralConfigDefine::Large;
                setPanelSize(92);
                break;
            case GeneralConfigDefine::Custom:
                m_sizePolicy = GeneralConfigDefine::Custom;
                setPanelSize(mainView()->config()->getValue(QStringLiteral("customPanelSize")).toInt());
                break;
        }

        mainView()->config()->setValue(QStringLiteral("panelSizePolicy"), m_sizePolicy);
        emit enableCustomSizeChanged();
    }

    if (m_sizeAction) {
        updateSizeAction();
    }
}

void Panel::widgetActionTriggered(const QAction *action)
{
    QString id = action->property("widget").toString();
    if (id.isEmpty()) {
        return;
    }

    disableWidget(id, !action->isChecked());
    mainView()->config()->setValue(QStringLiteral("disabledWidgets"), m_disabledWidgets);
}

void Panel::disableWidget(const QString &id, bool disable)
{
    if (disable) {
        // 禁用
        if (m_disabledWidgets.contains(id)) {
            return;
        }
        m_disabledWidgets.append(id);

        // 卸载全部widget
        QVector<LingmoUIQuick::Widget *> widgets = mainView()->widgets();
        QList<int> instances;
        for (const auto &item: widgets) {
            if (item->id() == id) {
                instances.append(item->instanceId());
            }
        }
        instances.append(mainView()->disableInstances());
        mainView()->setDisableInstances(instances);
    } else {
        // 启用
        if (m_disabledWidgets.removeAll(id) == 0) {
            return;
        }

        // 重新激活全部widget
        LingmoUIQuick::ConfigList children = mainView()->config()->children(QStringLiteral("widgets"));
        for (const auto &child : children) {
            const QString widgetId = child->getValue(QStringLiteral("id")).toString();
            if (widgetId == id) {
                mainView()->removeDisableInstance(child->id().toInt());
                mainView()->addWidget(widgetId, child->id().toInt());
            }
        }
    }

    for (const auto &action: m_widgetActions) {
        if (action->property("widget").toString() == id) {
            action->setChecked(!disable);
            break;
        }
    }
}

void Panel::loadActions()
{
    QList<QAction *> actions;
    // widget操作
    actions << m_widgetActions;

    // 分割线
    auto *separator = new QAction(this);
    separator->setSeparator(true);
    actions << separator;

    auto showDesktop = new QAction(tr("Show Desktop"), this);
    connect(showDesktop, &QAction::triggered, this, [] {
        KWindowSystem::setShowingDesktop(!KWindowSystem::showingDesktop());
    });
    actions << showDesktop;

    auto systemMonitor = new QAction(tr("System Monitor"), this);
    connect(systemMonitor, &QAction::triggered, this, [] {
        LingmoUIQuick::AppLauncher::instance()->launchApp(QStringLiteral("/usr/share/applications/lingmo-system-monitor.desktop"));
    });
    actions << systemMonitor;

    // 分割线
    auto *separator2 = new QAction(this);
    separator2->setSeparator(true);
    actions << separator2;

    // actions
    initSizeAction();
    initPositionAction();
    initLockPanelAction();

    // 调整大小
    actions << m_sizeAction;
    // 调整位置
    actions << m_positionAction;
    actions << m_lockPanelAction;

    // 自动隐藏
    m_autoHideAction = new QAction(tr("Auto Hide"), this);
    m_autoHideAction->setCheckable(true);
    m_autoHideAction->setChecked(m_autoHide);
    connect(m_autoHideAction, &QAction::triggered, this, [this] {
        setAutoHide(m_autoHideAction->isChecked());
    });

    actions << m_autoHideAction;

    // auto switchPanel = new QAction(tr("Switch to New Panel"), this);
    // connect(switchPanel, &QAction::triggered, this, [] {
    //     QTimer::singleShot(1, [](){
    //         Shell::self()->switchTo(Shell::PanelType::Version5);
    //     });
    // });
    // actions << switchPanel;

    auto panelSetting = new QAction(tr("Panel Setting"), this);
    connect(panelSetting, &QAction::triggered, this, [] {
        LingmoUIQuick::AppLauncher::instance()->launchAppWithArguments(QStringLiteral("/usr/share/applications/lingmo-control-center.desktop"), {"-m", "Panel"});
    });
    actions << panelSetting;

    mainView()->setActions(actions);
}

void Panel::initSizeAction()
{
    auto *sizeAction = new QAction(tr("Panel Size"), this);
    sizeAction->setMenu(new QMenu());

    QAction *action = sizeAction->menu()->addAction(tr("Large"));
    action->setProperty("sizePolicy", GeneralConfigDefine::Large);

    action = sizeAction->menu()->addAction(tr("Medium"));
    action->setProperty("sizePolicy", GeneralConfigDefine::Medium);

    action = sizeAction->menu()->addAction(tr("Small"));
    action->setProperty("sizePolicy", GeneralConfigDefine::Small);

    action = sizeAction->menu()->addAction(tr("Custom"));
    action->setProperty("sizePolicy", GeneralConfigDefine::Custom);

    connect(sizeAction->menu(), &QMenu::triggered, this, [this](QAction *action) {
        auto sizePolicy = action->property("sizePolicy").value<GeneralConfigDefine::PanelSizePolicy>();
        setPanelPolicy(sizePolicy);
    });

    m_sizeAction = sizeAction;
    updateSizeAction();
}

void Panel::updateSizeAction()
{
    for (const auto &action: m_sizeAction->menu()->actions()) {
        action->setCheckable(true);
        auto sp = action->property("sizePolicy").value<GeneralConfigDefine::PanelSizePolicy>();
        action->setChecked(sp == m_sizePolicy);
    }

    m_sizeAction->setEnabled(!m_lockPanel);
}

void Panel::initPositionAction()
{
    auto *positionAction = new QAction(tr("Panel Position"), this);
    positionAction->setMenu(new QMenu());

    QAction *action = positionAction->menu()->addAction(tr("Top"));
    action->setProperty("position", LingmoUIQuick::Types::Top);

    action = positionAction->menu()->addAction(tr("Bottom"));
    action->setProperty("position", LingmoUIQuick::Types::Bottom);

    action = positionAction->menu()->addAction(tr("Left"));
    action->setProperty("position", LingmoUIQuick::Types::Left);

    action = positionAction->menu()->addAction(tr("Right"));
    action->setProperty("position", LingmoUIQuick::Types::Right);

    connect(positionAction->menu(), &QMenu::triggered, this, [this](QAction *action) {
        auto pos = action->property("position").value<LingmoUIQuick::Types::Pos>();
        setPosition(pos);
    });

    m_positionAction = positionAction;

    updatePositionAction();
}

void Panel::updatePositionAction()
{
    for (const auto &action: m_positionAction->menu()->actions()) {
        action->setCheckable(true);
        action->setChecked(action->property("position").value<LingmoUIQuick::Types::Pos>() == mainView()->position());
    }

    m_positionAction->setEnabled(!m_lockPanel);
}

void Panel::initLockPanelAction()
{
    m_lockPanelAction = new QAction(tr("Lock Panel"), this);
    m_lockPanelAction->setCheckable(true);

    connect(m_lockPanelAction, &QAction::triggered, this, [this] {
        setLockPanel(m_lockPanelAction->isChecked());
    });

    updateLockPanelAction();
}

void Panel::updateLockPanelAction()
{
    m_lockPanelAction->setChecked(m_lockPanel);
    m_sizeAction->setEnabled(!m_lockPanel);
    m_positionAction->setEnabled(!m_lockPanel);
}

void Panel::resetWorkArea(bool cut)
{
    if (cut) {
        // 为窗管添加不可显示区域
        KWindowSystem::setExtendedStrut(winId(),
                                        m_unavailableArea.leftWidth, m_unavailableArea.leftStart, m_unavailableArea.leftEnd,
                                        m_unavailableArea.rightWidth, m_unavailableArea.rightStart, m_unavailableArea.rightEnd,
                                        m_unavailableArea.topWidth, m_unavailableArea.topStart, m_unavailableArea.topEnd,
                                        m_unavailableArea.bottomWidth, m_unavailableArea.bottomStart, m_unavailableArea.bottomEnd
        );

        m_windowProxy->setPanelAutoHide(false);
    } else {
        // 为窗管移除不可显示区域
        KWindowSystem::setExtendedStrut(winId(), 0, 0, 0, 0, 0, 0,
                                        0, 0, 0, 0, 0, 0
        );

        m_windowProxy->setPanelAutoHide(true);
    }
}

/**
 * 设置panel是否自动隐藏
 * 如果自动隐藏，那么需要向窗管归还可用区域
 * 将任务栏调整为屏幕边缘的一个小条
 * 并且将内容隐藏
 *
 * @param autoHide
 */
void Panel::setAutoHide(bool autoHide)
{
    if (m_autoHide == autoHide) {
        return;
    }

    m_autoHide = autoHide;

    activeHideTimer(false);
    setHidden(m_autoHide);
    resetWorkArea(!m_autoHide);

    if (m_autoHideAction) {
        m_autoHideAction->setChecked(m_autoHide);
    }

    mainView()->config()->setValue(QStringLiteral("panelAutoHide"), m_autoHide);
    QGSettings settings("org.lingmo.panel.settings", "/org/lingmo/panel/settings/", this);
    settings.set(QStringLiteral("hidepanel"), m_autoHide);
}

bool Panel::event(QEvent *e)
{
    switch (e->type()) {
        case QEvent::Leave: {
            m_containsMouse = false;
            if (m_autoHide && !m_isHidden && !mainView()->active()) {
                activeHideTimer();
            }
            break;
        }
        case QEvent::Enter: {
            m_containsMouse = true;
            if (m_autoHide) {
                activeHideTimer(false);
                setHidden(false);
            }
            break;
        }
        default:
            break;
    }

    return QQuickWindow::event(e);
}

void Panel::customPanelSize(int distance)
{
    if (m_sizePolicy == GeneralConfigDefine::Custom) {
        setPanelSize(m_panelSize + distance);
    }
}

void Panel::changeCursor(const QString &type)
{
    if (type.isEmpty()) {
        QGuiApplication::restoreOverrideCursor();
        return;
    }

    Qt::CursorShape shape = Qt::CustomCursor;
    if (type == "resizeVec") {
        shape = Qt::SizeVerCursor;
    } else if (type == "resizeHor") {
        shape = Qt::SizeHorCursor;
    } else if (type == "move") {
        shape = Qt::SizeAllCursor;
    }

    if (shape == Qt::CustomCursor) {
        QGuiApplication::restoreOverrideCursor();
        return;
    }

    auto cursor = QGuiApplication::overrideCursor();
    if (cursor && cursor->shape() == shape) {
        return;
    }

    QGuiApplication::setOverrideCursor({shape});
}

bool Panel::enableCustomSize() const
{
    return m_sizePolicy == GeneralConfigDefine::Custom;
}

void Panel::changePanelPos(LingmoUIQuick::Types::Pos pos)
{
    setPosition(pos);
}

void Panel::setLockPanel(bool locked, bool noEmit)
{
    if (locked == m_lockPanel) {
        return;
    }

    m_lockPanel = locked;
    updateLockPanelAction();

    if (!noEmit) {
        emit lockPanelChanged();
    }

    mainView()->config()->setValue(QStringLiteral("lockPanel"), m_lockPanel);
    QGSettings settings("org.lingmo.panel.settings", "/org/lingmo/panel/settings/", this);
    settings.set(QStringLiteral("lockpanel"), m_lockPanel);
}

bool Panel::lockPanel() const
{
    return m_lockPanel;
}

void Panel::onContainerActiveChanged()
{
    if (mainView()->active()) {
        activeHideTimer(false);
        setHidden(false);
    } else {
        if (m_autoHide && !m_containsMouse) {
            activeHideTimer();
        }
    }
}

void Panel::updateMask()
{
    QRegion mask;
    if (m_isHidden) {
        const int maskSize = 4;
        switch (mainView()->position()) {
            case LingmoUIQuick::Types::Left:
                mask = QRegion(0, 0, maskSize, geometry().height());
                break;
            case LingmoUIQuick::Types::Top:
                mask = QRegion(0, 0, geometry().width(), maskSize);
                break;
            case LingmoUIQuick::Types::Right:
                mask = QRegion(geometry().width() - maskSize, 0, maskSize, geometry().height());
                break;
            case LingmoUIQuick::Types::Bottom:
                mask = QRegion(0, geometry().height() - maskSize, geometry().width(), maskSize);
                break;
            default:
                break;
        }

        KWindowEffects::enableBlurBehind(this, false);

    } else {
        // TODO: region圆角
        auto blurRegion = QRegion(QRect{QPoint{mainView()->margin()->left(), mainView()->margin()->top()}, mainView()->geometry().size()});
        KWindowEffects::enableBlurBehind(this, true, blurRegion);
    }

    setMask(mask);
}

/**
 * 进入隐藏状态，并在过程中执行过渡动画
 * 隐藏状态是指任务栏内容不可见，并在屏幕边缘保留有一个小小的条状区域用于接受鼠标事件，动态激活界面
 *
    基本逻辑分为以下两个部分
    if (hidden) {
         执行动画（由）
         关闭毛玻璃
         隐藏内容
         显示小条
    } else {
         开启毛玻璃
         显示内容
         执行显示动画
    }
 *
 * @param hidden
 */
void Panel::setHidden(bool hidden)
{
    auto panelContent = rootItem();
    if (!panelContent || m_isHidden == hidden) {
        return;
    }

    m_isHidden = hidden;

    hide();
    panelContent->setVisible(!m_isHidden);
    updateMask();
    show();
}

void Panel::activeHideTimer(bool active)
{
    if (!m_hideTimer) {
        return;
    }

    if (active) {
        m_hideTimer->start();
    } else {
        m_hideTimer->stop();
    }
}

void Panel::primaryScreenChanged(Screen *screen)
{
    if(m_screen == screen) {
        mainView()->addWidget(QStringLiteral("org.lingmo.systemTray"), m_systemTrayInstance);
    } else {
        if(mainView()->config()->getValue("showSystemTrayOnAllPanels").toBool()) {
            mainView()->addWidget(QStringLiteral("org.lingmo.systemTray"), m_systemTrayInstance);
        } else {
            mainView()->removeWidget(QStringLiteral("org.lingmo.systemTray"));
        }
    }
}

void Panel::setShowSystemTrayStatus()
{
    //读取托盘是否始终显示在任务栏上
    auto value = mainView()->config()->getValue(QStringLiteral("showSystemTrayOnAllPanels"));
    if (!value.isValid()) {
        value = true;
        mainView()->config()->setValue(QStringLiteral("showSystemTrayOnAllPanels"), value);
    }
    //获取系统托盘的instanceid
    if(m_systemTrayInstance == -1) {
        auto widgetsList = mainView()->config()->children(QStringLiteral("widgets"));
        for (const auto widgetConfig : widgetsList) {
            if(widgetConfig->getValue(QStringLiteral("id")).toString() == QStringLiteral("org.lingmo.systemTray")) {
                m_systemTrayInstance = widgetConfig->getValue(QStringLiteral("instanceId")).toInt();
                break;
            }
        }
    }
    if(m_screen->internal() != qApp->primaryScreen() && !value.toBool()) {
        mainView()->removeWidget(QStringLiteral("org.lingmo.systemTray"));
    }
}

} // LingmoUIPanel
