/*
 *
 *  * Copyright (C) 2023, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#include "lingmo-task-manager.h"
#include <QVector>
#include <QSettings>
#include <QHash>
#include <QVariant>
#include <QDir>
#include <QFile>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QTranslator>
#include <QScreen>

#include <qglobal.h>
#include "task-manager-item.h"
#include "taskmanageradaptor.h"
#include "utils.h"
#include <config-loader.h>
#include <window-manager.h>
#include "actions.h"
#include "config.h"
#include "taskmanageradaptor.h"
#include <lingmosdk/diagnosetest/libkydatacollect.h>

static const QString QUICK_LAUNCHERS_KEY("quickLaunchers");


static const QString NOTIFICATION_SERVICE = QStringLiteral("org.freedesktop.Notifications");
static const QString NOTIFICATION_PATH = QStringLiteral("/org/freedesktop/Notifications");
static const QString NOTIFICATION_INTERFACE = QStringLiteral("org.lingmo.NotificationServer");
static const QStringList DEFAULT_QUICK_LAUNCHERS = {"/usr/share/applications/explor.desktop",
                                                    "/usr/share/applications/firefox.desktop",
                                                    "/usr/share/applications/lingmo-software-center.desktop",
                                                    "/usr/share/applications/lingmo-screenshot.desktop",
                                                    "/usr/share/applications/lingmo-control-center.desktop",
                                                    };
namespace TaskManager
{
using WindowManager = LingmoUIQuick::WindowManager;

class Q_DECL_HIDDEN LingmoUITaskManager::Private
{
public:
    explicit Private(LingmoUITaskManager *q = nullptr);
    ~Private();

    void windowAdded(const QString &windowId);
    void windowRemoved(const QString &windowId);
    void skipTaskbarChanged(const QString& windowId);
    void activeWindowChanged(const QString& windowId);
    void loadSettings();
    void save() const;
    void loadWindows();
    void newItem(const QString &wid, const QString &desktopFile);
    void onNotificationServiceChanged(const QString &service, const QString &oldOwner, const QString &newOwner);
    void connectToNotificationServer();
    QModelIndex indexOf(const QString &desktopFile);

    QVector<TaskManagerItem *> m_items;

    LingmoUIQuick::Config *m_config {nullptr};
    QStringList m_quickLaunchersOrder;

    uint m_groupID = 0; //当窗口的group为空时使用
    QTranslator *m_translator = nullptr;
    QDBusServiceWatcher *m_watcher = nullptr;
    bool m_connectedToNotificationServer = false;
    QString m_activeWindow;
    TaskManager::Actions m_windowActions;

private:
    LingmoUITaskManager *q = nullptr;
};

LingmoUITaskManager::Private::Private(LingmoUITaskManager *q) : q(q)
{
}

LingmoUITaskManager::Private::~Private()
{
    qDeleteAll(m_items);
    qDeleteAll(m_windowActions);
}

void LingmoUITaskManager::Private::loadSettings()
{
    //TODO：兼容老版本配置文件
    if (!m_config) {
        m_config = LingmoUIQuick::ConfigLoader::getConfig("org.lingmo.panel.taskManager");
    }

    //设置默认值
    if (m_config->data().contains(QUICK_LAUNCHERS_KEY)) {
        m_quickLaunchersOrder = m_config->getValue(QUICK_LAUNCHERS_KEY).toStringList();
    } else {
        m_quickLaunchersOrder = QStringList {
            "/usr/share/applications/explor.desktop",
            "/usr/share/applications/firefox.desktop",
            "/usr/share/applications/lingmo-software-center.desktop",
            "/usr/share/applications/lingmo-screenshot.desktop",
            "/usr/share/applications/lingmo-control-center.desktop",
        };
    }

    //加载数据并初始化固定的按钮
    for(const QString &desktopFile : m_quickLaunchersOrder) {
        if(QFile::exists(desktopFile)) {
            auto item = new TaskManagerItem(desktopFile);
            connect(item, &TaskManagerItem::dataUpdated, q, [&, item](const QVector<int> &roles){
                QModelIndex changeIndex = q->index(m_items.indexOf(item), 0, QModelIndex());
                q->dataChanged(changeIndex, changeIndex, roles);
            });
            connect(q, &LingmoUITaskManager::unReadMessagesNumberUpdate, item, &TaskManagerItem::updateUnreadMessagesNum);
            item->setHasLauncher(true);
            item->init();
            q->beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
            m_items.append(item);
            q->endInsertRows();
        } else {
            m_quickLaunchersOrder.removeAll(desktopFile);
        }
    }

    m_config->setValue(QUICK_LAUNCHERS_KEY, m_quickLaunchersOrder);
    m_config->forceSync();
}

void LingmoUITaskManager::Private::windowAdded(const QString &windowId)
{
    //过滤不需要显示的窗口
    if(WindowManager::skipTaskBar(windowId)) {
        return;
    }
    QString windowGroup = WindowManager::windowGroup(windowId);
    if (windowGroup.isEmpty()) {
        windowGroup = QString::number(++m_groupID);
    }
    QString desktopFile =  Utils::desktopFileFromWid(windowId);
    for (auto item : m_items) {
        //避免重复添加
        if(item->winIDs().contains(windowId)) {
            return;
        }
        //组已经存在，增加一个窗口
        if ((!item->ID().isEmpty()
            && !desktopFile.isEmpty()
            && (item->ID() == desktopFile || QFileInfo(item->ID()).fileName() == QFileInfo(desktopFile).fileName())) || item->group() == windowGroup) {
            item->addWinID(windowId);
            return;
        }
    }
    newItem(windowId, desktopFile);
}

void LingmoUITaskManager::Private::windowRemoved(const QString &windowId)
{
    for (int index = 0; index < m_items.size(); ++index) {
        auto item = m_items.at(index);
        if (item->winIDs().contains(windowId)) {
            item->removeWinID(windowId);
            if (item->winIDs().isEmpty() && !item->hasLauncher()) {
                q->beginRemoveRows(QModelIndex(), index, index);
                delete m_items.takeAt(index);
                q->endRemoveRows();
            }
            break;
        }
    }
}

void LingmoUITaskManager::Private::skipTaskbarChanged(const QString &windowId)
{
    bool skipTaskBar = WindowManager::skipTaskBar(windowId);
    QString windowGroup = WindowManager::windowGroup(windowId);
    bool handled = false;
    for (int index = 0; index < m_items.size(); ++index) {
        auto item = m_items.at(index);
        if (item->winIDs().contains(windowId)) {
            if(skipTaskBar) {
                item->removeWinID(windowId);
                if (item->winIDs().isEmpty() && !item->hasLauncher()) {
                    q->beginRemoveRows(QModelIndex(), index, index);
                    delete m_items.takeAt(index);
                    q->endRemoveRows();
                }
            }
            handled = true;
            break;
        } else if(item->group() == windowGroup && !windowGroup.isEmpty() && !skipTaskBar) {
            item->addWinID(windowId);
            handled = true;
            break;
        }
    }
    if(!handled) {
        if(!skipTaskBar) {
            newItem(windowId, Utils::desktopFileFromWid(windowId));
        }
    }
}

void LingmoUITaskManager::Private::activeWindowChanged(const QString &windowId)
{
    if(windowId == m_activeWindow) {
        return;
    }
    for(auto item : m_items) {
        if(item->currentDesktopWinIDs().contains(m_activeWindow)) {
            item->setHasActiveWindow(false);
        }
        if(item->currentDesktopWinIDs().contains(windowId)) {
            item->setHasActiveWindow(true);
        }
    }
    m_activeWindow = windowId;
}

void LingmoUITaskManager::Private::loadWindows()
{
    for (const QString &winID: WindowManager::windows()) {
        windowAdded(winID);
    }
}

void LingmoUITaskManager::Private::newItem(const QString &wid, const QString &desktopFile)
{
    //查看是否存在快速启动按钮
    if (m_quickLaunchersOrder.contains(desktopFile)) {
        m_items.at(indexOf(desktopFile).row())->addWinID(wid);
        return;
    }

    //新增一个item
    auto newItem = new TaskManagerItem(QStringList{wid});
    newItem->setID(desktopFile);
    newItem->init();

    connect(q, &LingmoUITaskManager::unReadMessagesNumberUpdate, newItem, &TaskManagerItem::updateUnreadMessagesNum);

    QObject::connect(newItem, &TaskManagerItem::dataUpdated, q, [&, newItem](const QVector<int> &roles){
        QModelIndex changeIndex = q->index(m_items.indexOf(newItem), 0, QModelIndex());
        q->dataChanged(changeIndex, changeIndex, roles);
    });
    q->beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
    m_items.append(newItem);
    q->endInsertRows();
}

void LingmoUITaskManager::Private::save() const
{
    m_config->setValue(QUICK_LAUNCHERS_KEY, m_quickLaunchersOrder);
    m_config->forceSync();
}

void LingmoUITaskManager::Private::onNotificationServiceChanged(const QString &service, const QString &oldOwner,
                                                            const QString &newOwner)
{
    if(!oldOwner.isEmpty()) {
        QDBusConnection::sessionBus().disconnect(NOTIFICATION_SERVICE,
                                                 NOTIFICATION_PATH,
                                                 NOTIFICATION_INTERFACE,
                                                 QStringLiteral("UnreadMessagesNumberUpdated"),
                                                 q,
                                                 SLOT(unReadMessagesNumberUpdate));
        m_connectedToNotificationServer = false;
        if(!newOwner.isEmpty()) {
            connectToNotificationServer();
        }
    } else if(!newOwner.isEmpty()) {
        if(!m_connectedToNotificationServer) {
            return;
        }
        connectToNotificationServer();
    }
}

void LingmoUITaskManager::Private::connectToNotificationServer()
{
    m_connectedToNotificationServer = QDBusConnection::sessionBus().connect(NOTIFICATION_SERVICE,
                                                                            NOTIFICATION_PATH,
                                                                            NOTIFICATION_INTERFACE,
                                                                            QStringLiteral("UnreadMessagesNumberUpdated"),
                                                                            q,
                                                                            SLOT(unReadMessagesNumberUpdate(const QString &, uint)));
}

QModelIndex LingmoUITaskManager::Private::indexOf(const QString &desktopFile)
{
    for(int row = 0; row <= m_items.size(); ++row) {
        if(m_items.at(row)->ID() == desktopFile) {
            return q->index(row, 0, {});
        }
    }
    return {};
}
LingmoUITaskManager::LingmoUITaskManager(QObject *parent) : QAbstractListModel(parent), d(new Private(this))
{
    d->m_translator = new QTranslator(this);
    if (!d->m_translator->load(QString(TRANSLATION_FILE_DIR) + "/liblingmo-task-manager_" + QLocale::system().name() + ".qm")) {
        qWarning() << "Load translations file" << QLocale::system().name() << "failed!";;
    }
    qApp->installTranslator(d->m_translator);
    new TaskManagerAdaptor(this);
    //加载配置文件，初始化快速启动按钮
    d->loadSettings();
    //加载当前打开的窗口
    d->loadWindows();
    connect(Utils::getApplicationInfo(), &LingmoUISearch::ApplicationInfo::appDBItems2BDelete, this, &LingmoUITaskManager::onAppUninstalled);
    connect(WindowManager::self(), &WindowManager::windowAdded, this, [&](const QString &winId) {
        d->windowAdded(winId);
    });
    connect(WindowManager::self(), &WindowManager::windowRemoved, this, [&](const QString &winId) {
        d->windowRemoved(winId);
    });
    connect(WindowManager::self(), &WindowManager::skipTaskbarChanged, this, [&](const QString &winId) {
        d->skipTaskbarChanged(winId);
    });
    connect(WindowManager::self(), &WindowManager::activeWindowChanged, this, [&](const QString &winId){
        d->activeWindowChanged(winId);
    });
    d->m_activeWindow = WindowManager::currentActiveWindow();

    auto conn = QDBusConnection::sessionBus();
    d->m_watcher = new QDBusServiceWatcher(QStringLiteral("org.freedesktop.Notifications"),
                                           conn,
                                           QDBusServiceWatcher::WatchForOwnerChange,
                                           this);
    connect(d->m_watcher, &QDBusServiceWatcher::serviceOwnerChanged,this,[&](const QString &service, const QString &oldOwner, const QString &newOwner){
        d->onNotificationServiceChanged(service, oldOwner, newOwner);
            });
    d->connectToNotificationServer();
    new TaskManagerAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject(QStringLiteral("/taskManager"), this);
    dbus.registerService(QStringLiteral("org.lingmo.taskManager"));
}

LingmoUITaskManager::~LingmoUITaskManager()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

QHash<int, QByteArray> LingmoUITaskManager::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    QMetaEnum e = metaObject()->enumerator(metaObject()->indexOfEnumerator("AdditionalRoles"));

    for (int i = 0; i < e.keyCount(); ++i) {
        roles.insert(e.value(i), e.key(i));
    }
    return roles;
}

QVariant LingmoUITaskManager::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= d->m_items.count()) {
        return {};
    }
    TaskManagerItem *item = d->m_items.at(index.row());

    if (!item) {
        return {};
    }
    switch (role) {
        case Qt::DisplayRole:
            return item->name();
        case Icon: {
            if (item->icon().isNull()) {
                return "application-x-desktop";
            }
            return item->icon();
        }
        case Id:
            return item->ID();
        case Name:
            return item->name();
        case GenericName:
            return item->genericName();
        case WinIdList:
            return item->winIDs();
        case CurrentDesktopWinIdList:
            return item->currentDesktopWinIDs();
        case WinIdsOnScreens:
            return QVariant::fromValue(item->winIdOnScreens());
        case WindowTitles:
            return QVariant::fromValue(item->windowTitles());
        case WindowIcons:
            return QVariant::fromValue(item->windowIcons());
        case DemandsAttentionWinIdList:
            return item->demandsAttentionWinIDs();
        case ApplicationMenuServiceName:
        case ApplicationMenuObjectPath:
            return QString();
        case HasLauncher:
            return item->hasLauncher();
        case Actions: {
            auto actions = item->actions();
            for(auto action : actions) {
                if(action->type() == Action::Type::AddQuickLauncher) {
                    action->setParam(quickLauncherCountBeforeRow(index.row()));
                    break;
                }
            }
            return QVariant::fromValue(actions);
        }

        case UnreadMessagesNum:
            return item->unReadMessagesNum();
        case HasActiveWindow:
            return item->hasActiveWindow();
        default:
            return {};
    }
    Q_UNREACHABLE();
}

QModelIndex LingmoUITaskManager::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column != 0) {
        return {};
    }
    return createIndex(row, column, nullptr);
}

int LingmoUITaskManager::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : d->m_items.count();
}

bool LingmoUITaskManager::addQuickLauncher(const QString &desktopFile)
{
    if (!QFile::exists(desktopFile) || d->m_quickLaunchersOrder.contains(desktopFile)) {
        return false;
    }
    addQuickLauncher(desktopFile, d->m_quickLaunchersOrder.size());
    return true;
}

bool LingmoUITaskManager::checkQuickLauncher(const QString &desktopFile)
{
    return d->m_quickLaunchersOrder.contains(desktopFile);
}

bool LingmoUITaskManager::removeQuickLauncher(const QString &desktopFile)
{
    if (!QFile::exists(desktopFile) || !d->m_quickLaunchersOrder.contains(desktopFile)) {
        return false;
    }
    addQuickLauncher(desktopFile, -1);
    return true;
}

void LingmoUITaskManager::addQuickLauncher(const QString &desktopFile, int order)
{
    //改位置或取消固定
    if(d->m_quickLaunchersOrder.contains(desktopFile)) {
        int modelIndex = d->indexOf(desktopFile).row();
        auto targetItem = d->m_items.at(modelIndex);
        int oldOrder = d->m_quickLaunchersOrder.indexOf(desktopFile);
        if (oldOrder == order) {
            return;
        }
        //取消固定
        if (order < 0) {
            targetItem->setHasLauncher(false);
            //上传埋点数据
            addQuickLauncherEvent(desktopFile, "unpin from taskbar");

            if (targetItem->winIDs().isEmpty()) {
                beginRemoveRows(QModelIndex(), modelIndex, modelIndex);
                delete d->m_items.takeAt(modelIndex);
                endRemoveRows();
            }
            d->m_quickLaunchersOrder.removeAt(oldOrder);
        } else {
            if (order > d->m_quickLaunchersOrder.size() - 1) {
                order = d->m_quickLaunchersOrder.size() - 1;
            }
            d->m_quickLaunchersOrder.insert(order, d->m_quickLaunchersOrder.takeAt(oldOrder));
        }
        d->save();
    } else if (order >= 0) {        //添加新的按钮
        if (!QFile::exists(desktopFile)) {
            return;
        }
        int i = 0;
        for (; i < d->m_items.size(); ++i) {
            if(d->m_items.at(i)->ID() == desktopFile) {
                d->m_items.at(i)->setHasLauncher(true);
                break;
            }
        }
        //新增的图标
        if(i >= d->m_items.size()) {
            auto item = new TaskManagerItem(desktopFile);
            order = qMin(order, d->m_quickLaunchersOrder.size() - 1);
            item->setHasLauncher(true);

            connect(this, &LingmoUITaskManager::unReadMessagesNumberUpdate, item, &TaskManagerItem::updateUnreadMessagesNum);
            QObject::connect(item, &TaskManagerItem::dataUpdated, this, [&, item](const QVector<int> &roles){
                QModelIndex changeIndex = index(d->m_items.indexOf(item), 0, QModelIndex());
                Q_EMIT dataChanged(changeIndex, changeIndex, roles);
            });
            item->init();
            beginInsertRows(QModelIndex(), d->m_items.size(), d->m_items.size());
            d->m_items.append(item);
            endInsertRows();
        }
        d->m_quickLaunchersOrder.insert(order, desktopFile);
        d->save();
        //上传埋点数据
        addQuickLauncherEvent(desktopFile, "pin to taskbar");
    }
}

void LingmoUITaskManager::setOrder(const QModelIndex &index, int order)
{
    int oldOrder = index.row();
    if(order == oldOrder || !index.isValid() || oldOrder >= d->m_items.size()) {
        return;
    }
    auto item = d->m_items.at(oldOrder);
    if(item->hasLauncher()) {
        int launcherOrder = d->m_quickLaunchersOrder.indexOf(item->ID());
        if(oldOrder < order) {
            for(int row = oldOrder + 1; row <= order; ++row) {
                if(d->m_items.at(row)->hasLauncher()) {
                    ++launcherOrder;
                }
            }
        } else {
            for(int row = oldOrder - 1; row >= order; --row) {
                if(d->m_items.at(row)->hasLauncher()) {
                    --launcherOrder;
                }
            }

            if (launcherOrder < 0) {
                launcherOrder = 0;
            }
        }
        addQuickLauncher(item->ID(), launcherOrder);
    }

    if (oldOrder < order) {
        beginMoveRows(index.parent(), oldOrder, oldOrder, index.parent(), order + 1);
    } else {
        beginMoveRows(index.parent(), oldOrder, oldOrder, index.parent(), order);
    }

    d->m_items.insert(order, d->m_items.takeAt(oldOrder));
    endMoveRows();
}

void LingmoUITaskManager::activateWindowView(const QStringList &winIds)
{
    WindowManager::self()->activateWindowView(winIds);
}

void LingmoUITaskManager::activateWindow(const QString &winId)
{
    WindowManager::activateWindow(winId);
}

QString LingmoUITaskManager::windowTitle(const QString &winId)
{
    return WindowManager::windowTitle(winId);
}

QIcon LingmoUITaskManager::windowIcon(const QString &winId)
{
    return WindowManager::windowIcon(winId);
}

LingmoUITaskManager &LingmoUITaskManager::self()
{
    static LingmoUITaskManager s_self;
    return s_self;
}

TaskManager::Actions LingmoUITaskManager::windowActions(const QString &winId)
{
    if(d->m_windowActions.isEmpty()) {
        //关闭
        auto action = new Action("Close", tr("Close"), QIcon::fromTheme("window-close-symbolic"), Action::Type::Close, winId);
        connect(action, &Action::actionTriggered, this, &LingmoUITaskManager::closeWindow);
        d->m_windowActions.append(action);
        //还原
        action = new Action("Restore", tr("Restore"), QIcon::fromTheme("window-restore-symbolic"), Action::Type::Restore, winId);
        connect(action, &Action::actionTriggered, this, &LingmoUITaskManager::restoreWindow);
        d->m_windowActions.append(action);
        //最大化
        action = new Action("Maximize", tr("Maximize"), QIcon::fromTheme("window-maximize-symbolic"), Action::Type::Maximize, winId);
        connect(action, &Action::actionTriggered, this, &LingmoUITaskManager::maximizeWindow);
        d->m_windowActions.append(action);
        //最小化
        action = new Action("Minimize", tr("Minimize"), QIcon::fromTheme("window-minimize-symbolic"), Action::Type::Minimize, winId);
        connect(action, &Action::actionTriggered, this, &LingmoUITaskManager::minimizeWindow);
        d->m_windowActions.append(action);
        //置顶
        action = new Action("Keep above", tr("Keep above"), QIcon::fromTheme("lingmo-fixed-symbolic"), Action::Type::KeepAbove, winId);
        connect(action, &Action::actionTriggered, this, &LingmoUITaskManager::keepAbove);
        d->m_windowActions.append(action);
        //取消置顶
        action = new Action("Unset keep above", tr("Unset keep above"), QIcon::fromTheme("lingmo-unfixed-symbolic"), Action::Type::UnsetKeepAbove, winId);
        connect(action, &Action::actionTriggered, this, &LingmoUITaskManager::unsetKeepAbove);
        d->m_windowActions.append(action);
    }
    for (auto action : d->m_windowActions) {
        if (action->param() != winId) {
            action->setParam(winId);
        }

        switch (action->type()) {
            case Action::Close:
                break;
            case Action::Minimize:
                action->setEnabled(WindowManager::isMinimizable(winId) && !WindowManager::isMinimized(winId));
                break;
            case Action::Maximize:
                action->setEnabled(WindowManager::isMaximizable(winId) && (!WindowManager::isMaximized(winId) || WindowManager::isMinimized(winId)));
                break;
            case Action::Restore:
                action->setEnabled(WindowManager::isMaximized(winId) || WindowManager::isMinimized(winId));
                break;
            case Action::KeepAbove:
                action->setEnabled(!WindowManager::isKeepAbove(winId));
                break;
            case Action::UnsetKeepAbove:
                action->setEnabled(WindowManager::isKeepAbove(winId));
                break;
            default:
                break;
        }
    }
    return d->m_windowActions;
}

bool LingmoUITaskManager::windowIsActivated(const QString &winId)
{
    return winId == WindowManager::currentActiveWindow();
}

void LingmoUITaskManager::execSpecifiedAction(const TaskManager::Action::Type &type, const QString &wid)
{
    switch (type) {
        case Action::Close:
            WindowManager::closeWindow(wid);
            break;
        case Action::Minimize:
            WindowManager::minimizeWindow(wid);
            break;
        case Action::Maximize:
            WindowManager::maximizeWindow(wid);
            break;
        case Action::Restore:
            WindowManager::restoreWindow(wid);
            break;
        case Action::KeepAbove:
        case Action::UnsetKeepAbove:
            WindowManager::keepAboveWindow(wid);
            break;
        default:
            break;
    }
}

void LingmoUITaskManager::closeWindow(const QVariant &wid)
{
    WindowManager::closeWindow(wid.toString());
}

void LingmoUITaskManager::minimizeWindow(const QVariant &wid)
{
    WindowManager::minimizeWindow(wid.toString());
}

void LingmoUITaskManager::maximizeWindow(const QVariant &wid)
{
    WindowManager::maximizeWindow(wid.toString());
    WindowManager::activateWindow(wid.toString());
}

void LingmoUITaskManager::restoreWindow(const QVariant &wid)
{
    WindowManager::restoreWindow(wid.toString());
    WindowManager::activateWindow(wid.toString());
}

void LingmoUITaskManager::keepAbove(const QVariant &wid)
{
    WindowManager::keepAboveWindow(wid.toString());
}

void LingmoUITaskManager::unsetKeepAbove(const QVariant &wid)
{
    WindowManager::keepAboveWindow(wid.toString());
}

void LingmoUITaskManager::launch(const QModelIndex &index)
{
    if (!index.isValid() || index.row() >= d->m_items.count()) {
        return;
    }
     d->m_items.at(index.row())->newInstanceActionActive();
}

int LingmoUITaskManager::quickLauncherCountBeforeRow(int row) const
{
    int launcherCount = 0;
    for(int i = 0; i < row; ++i) {
        if(d->m_items.at(i)->hasLauncher()) {
            launcherCount++;
        }
    }
    return launcherCount;
}

void LingmoUITaskManager::setQuickLauncher(const QString &desktopFile, int order)
{
    if(order >= d->m_items.size()) {
        order = d->m_items.size();
    }
    if(order >= 0) {
        order = quickLauncherCountBeforeRow(order);
    }
    addQuickLauncher(desktopFile, order);
}

void LingmoUITaskManager::onAppUninstalled(const QStringList &desktopFiles)
{
    for(const QString &desktopFile : desktopFiles) {
        if(d->m_quickLaunchersOrder.contains(desktopFile)) {
            addQuickLauncher(desktopFile, -1);
        }
    }
}

void LingmoUITaskManager::addQuickLauncherEvent(const QString &applicationName, const QString &operate )
{
    KCustomProperty *property = new KCustomProperty[2];
    property[0].key = strdup(QStringLiteral("AppliciationName").toLocal8Bit().data());
    property[0].value = strdup(applicationName.toLocal8Bit().data());
    property[1].key = strdup(QStringLiteral("EventType").toLocal8Bit().data());
    property[1].value = strdup(operate.toLocal8Bit().data());
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
    kdk_dia_append_custom_property(node, property, 2);
    kdk_dia_upload_default(node, strdup(QStringLiteral("pin_to_taskbar_event").toLocal8Bit().data()),strdup(QStringLiteral("lingmo-panel").toLocal8Bit().data()));
    kdk_dia_data_free(node);
    free(property[0].key);
    free(property[0].value);
    free(property[1].key);
    free(property[1].value);
    delete []property;
}

bool LingmoUITaskManager::event(QEvent *event)
{
    if (event->type() == QEvent::UpdateRequest) {
        for(auto item : d->m_items) {
            item->updateIconStatus();
        }
        m_requestDataChange = false;
    }
    return QObject::event(event);
}

void LingmoUITaskManager::requestDataChange()
{
    if (!m_requestDataChange) {
        m_requestDataChange = true;
        auto ev = new QEvent(QEvent::UpdateRequest);
        QCoreApplication::postEvent(this, ev);
    }
}
}
#include "moc_lingmo-task-manager.cpp"
