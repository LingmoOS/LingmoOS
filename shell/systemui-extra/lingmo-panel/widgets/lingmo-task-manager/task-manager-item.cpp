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

#include "task-manager-item.h"

#include <XdgDesktopFile>
#include <QDebug>
#include <QMap>
#include <QScreen>
#include <QGuiApplication>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusConnection>
#include "window-manager.h"
#include "lingmo-task-manager.h"
#include "utils.h"
using namespace TaskManager;
using namespace LingmoUIQuick;
class TaskManager::TaskManagerItemPrivate
{
public:
    ~TaskManagerItemPrivate();
    QString m_ID;
    QString m_name;
    QString m_genericName;
    QIcon m_icon;
    QStringList m_winIDs;
    QStringList m_winIDsOnCurrentDesktop;
    QString m_groupLeader;
    QString m_group;
    bool m_hasLauncher = false;
    QString m_display;
    Actions m_actions;
    XdgDesktopFile m_xdf;
    uint m_unreadMessagesNum = 0;
    QStringList m_demandsAttentionWinIDs;
    QMap<QString, QScreen *> m_widToScreen;
    QMap<QString, QVariant> m_widToTitle;
    QMap<QString, QVariant> m_widToIcon;
    bool m_hasActiveWindow = false;
};

TaskManagerItemPrivate::~TaskManagerItemPrivate()
{
    qDeleteAll(m_actions);
}

TaskManagerItem::TaskManagerItem(QObject *parent) : QObject(parent), d(new TaskManagerItemPrivate)
{
}

TaskManagerItem::TaskManagerItem(const QString &appID, QObject *parent): QObject(parent), d(new TaskManagerItemPrivate)
{
    d->m_ID = appID;
    d->m_xdf.load(appID);
}

TaskManagerItem::TaskManagerItem(const QStringList &winIDs, QObject *parent) : QObject(parent), d(new TaskManagerItemPrivate)
{
    d->m_winIDs = winIDs;
}

TaskManagerItem::~TaskManagerItem()
{
    if(d) {
        delete d;
        d = nullptr;
    }
}

const QString &TaskManagerItem::ID() const
{
    return d->m_ID;
}

void TaskManagerItem::setID(const QString &appID)
{
    d->m_ID = appID;
    d->m_xdf.load(appID);
}

bool TaskManagerItem::hasLauncher() const
{
    return d->m_hasLauncher;
}

void TaskManagerItem::setHasLauncher(bool hasLauncher)
{
    if(d->m_hasLauncher && hasLauncher) {
        return;
    }
    if(!hasLauncher && d->m_hasLauncher) {
        for(Action *action : d->m_actions) {
            if(action->type() == Action::Type::RemoveQuickLauncher) {
                action->setEnabled(false);
            }
            if(action->type() == Action::Type::AddQuickLauncher) {
                action->setEnabled(true);
            }
        }
    } else if(hasLauncher && !d->m_hasLauncher) {
        for(Action *action : d->m_actions) {
            if(action->type() == Action::Type::RemoveQuickLauncher) {
                action->setEnabled(true);
            }
            if(action->type() == Action::Type::AddQuickLauncher) {
                action->setEnabled(false);
            }
        }
    }
    d->m_hasLauncher = hasLauncher;
    Q_EMIT dataUpdated(QVector<int>{LingmoUITaskManager::HasLauncher, LingmoUITaskManager::Actions});
}

const QString &TaskManagerItem::display() const
{
    return d->m_display;
}

const QString &TaskManagerItem::name() const
{
    return d->m_name;
}

const QIcon &TaskManagerItem::icon() const
{
    return d->m_icon;
}

const QString &TaskManagerItem::genericName() const
{
    return d->m_genericName;
}

const QStringList &TaskManagerItem::winIDs() const
{
    return d->m_winIDs;
}

void TaskManagerItem::addWinID(const QString &winId)
{
    if(d->m_winIDs.contains(winId)) {
        return;
    }
    d->m_winIDs.append(winId);
    Q_EMIT dataUpdated(QVector<int>{LingmoUITaskManager::WinIdList});
    if(WindowManager::isOnCurrentDesktop(winId) || WindowManager::isOnAllDesktops(winId)) {
        d->m_winIDsOnCurrentDesktop.append(winId);
        Q_EMIT dataUpdated(QVector<int>{LingmoUITaskManager::CurrentDesktopWinIdList, LingmoUITaskManager::CurrentWinIdList});
    }
    geometryChanged(winId);
    QString title = WindowManager::windowTitle(winId);
    if(title.isEmpty()) {
        title = d->m_name;
    }
    d->m_widToTitle.insert(winId, title);
    QIcon icon = WindowManager::windowIcon(winId);
    if(icon.isNull()) {
        icon = d->m_icon;
    }
    d->m_widToIcon.insert(winId, icon.isNull() ? QVariant("application-x-desktop") : icon);
    Q_EMIT dataUpdated(QVector<int>{LingmoUITaskManager::WindowTitles, LingmoUITaskManager::WindowIcons});
}

void TaskManagerItem::removeWinID(const QString &winID)
{
    bool updateIconName = d->m_winIDs.isEmpty()? false : d->m_winIDs.first() == winID;
    if(!d->m_winIDs.isEmpty()) {
        d->m_winIDs.removeAll(winID);
        d->m_widToTitle.remove(winID);
        d->m_widToIcon.remove(winID);
        Q_EMIT dataUpdated(QVector<int>{LingmoUITaskManager::WinIdList, LingmoUITaskManager::WindowTitles, LingmoUITaskManager::WindowIcons});
        if(!hasLauncher() && d->m_xdf.name().isEmpty() && !d->m_winIDs.isEmpty() && updateIconName) {
            d->m_name = WindowManager::windowTitle(d->m_winIDs.first());
            d->m_icon = WindowManager::windowIcon(d->m_winIDs.first());
            Q_EMIT dataUpdated(QVector<int>{LingmoUITaskManager::Name, LingmoUITaskManager::Icon});
        }
    }

    if(d->m_winIDsOnCurrentDesktop.removeAll(winID)) {
        Q_EMIT dataUpdated(QVector<int>{LingmoUITaskManager::CurrentDesktopWinIdList, LingmoUITaskManager::CurrentWinIdList});
    }
    if(d->m_widToScreen.remove(winID)) {
        Q_EMIT dataUpdated(QVector<int>{LingmoUITaskManager::WinIdsOnScreens, LingmoUITaskManager::CurrentWinIdList});
    }
}

void TaskManagerItem::setHasActiveWindow(bool has)
{
    if(d->m_hasActiveWindow != has) {
        d->m_hasActiveWindow = has;
        Q_EMIT dataUpdated(QVector<int>{LingmoUITaskManager::HasActiveWindow});
    }
}

bool TaskManagerItem::hasActiveWindow()
{
    return d->m_hasActiveWindow;
}

const QString &TaskManagerItem::group() const
{
    return d->m_group;
}

void TaskManagerItem::setGroup(const QString &group)
{
    d->m_group = group;
}

void TaskManagerItem::init()
{
    connect(WindowManager::self(), &WindowManager::currentDesktopChanged, this, &TaskManagerItem::refreshWinIdsOnCurrentDesktop);
    connect(WindowManager::self(), &WindowManager::onAllDesktopsChanged, this, &TaskManagerItem::desktopChanged);
    connect(WindowManager::self(), &WindowManager::windowDesktopChanged, this, &TaskManagerItem::desktopChanged);
    connect(WindowManager::self(), &WindowManager::titleChanged, this, &TaskManagerItem::titleChanged);
    connect(WindowManager::self(), &WindowManager::iconChanged, this, &TaskManagerItem::iconChanged);
    connect(WindowManager::self(), &WindowManager::demandsAttentionChanged, this, &TaskManagerItem::demandsAttentionChanged);
    connect(WindowManager::self(), &WindowManager::geometryChanged, this, &TaskManagerItem::geometryChanged);
    connect(qApp, &QGuiApplication::screenAdded, this, &TaskManagerItem::refreshWinIdsOnScreens);
    connect(qApp, &QGuiApplication::screenRemoved, this, &TaskManagerItem::refreshWinIdsOnScreens);

    d->m_name = d->m_xdf.localizedValue("Name").toString();
    d->m_icon = QIcon::fromTheme(d->m_xdf.iconName());

    if(!winIDs().isEmpty()) {
        //TODO： 获取groupLeader窗口的图标？
        if(d->m_icon.isNull()) {
            d->m_icon = WindowManager::windowIcon(winIDs().at(0));
        }
        if(d->m_name.isEmpty()) {
            d->m_name = WindowManager::windowTitle(winIDs().at(0));
        }
        d->m_group =  WindowManager::windowGroup(winIDs().at(0));
        refreshWinIdsOnCurrentDesktop();
        refreshWinIdsOnScreens();
        refreshWindowTitlesAndIcons();
    }

    //添加Action
    //打开应用
    if(d->m_xdf.isValid()) {
        d->m_actions.append(new Action(d->m_xdf.name(), d->m_xdf.name(), d->m_icon, Action::Type::NewInstance));
        connect(d->m_actions.last(), &Action::actionTriggered, this, &TaskManagerItem::newInstanceActionActive);

    }
    //desktop中的action
    for(const QString &action : d->m_xdf.actions()) {
        auto a = new Action(action, d->m_xdf.actionName(action), d->m_xdf.actionIcon(action), Action::Type::DesktopAction, action);
        connect(a, &Action::actionTriggered, this, [&](const QVariant& param){
            desktopActionActive(param.toString());
        });
        d->m_actions.append(a);
    }
    //添加到任务栏和从任务栏移除
    auto removeLauncher = new Action("Remove launcher", tr("Remove launcher from panel"), QIcon::fromTheme("lingmo-unfixed-symbolic"), Action::Type::RemoveQuickLauncher);
    connect(removeLauncher, &Action::actionTriggered, this, &TaskManagerItem::removeQuickLauncherActionActive);

    auto addLauncher = new Action("Add launcher", tr("Add launcher to panel"), QIcon::fromTheme("lingmo-fixed-symbolic"), Action::Type::AddQuickLauncher);
    connect(addLauncher, &Action::actionTriggered, this, [&](const QVariant& param){
        addQuickLauncherActionActive(param.toInt());
    });
    removeLauncher->setEnabled(hasLauncher());
    addLauncher->setEnabled(!hasLauncher() && !d->m_ID.isEmpty());
    d->m_actions.append(removeLauncher);
    d->m_actions.append(addLauncher);

    //退出
    auto exit = new Action("Exit", tr("Exit"), QIcon::fromTheme("application-exit-symbolic"), Action::Type::Exit);
    connect(exit, &Action::actionTriggered, this, [&](const QVariant& param){
        exitActionActive(param.toStringList());
    });
    d->m_actions.append(exit);

    Q_EMIT dataUpdated(QVector<int>{LingmoUITaskManager::Actions,
                                    LingmoUITaskManager::Id,
                                    LingmoUITaskManager::Icon,
                                    LingmoUITaskManager::Name});
}

Actions TaskManagerItem::actions() const
{
    return d->m_actions;
}

QStringList TaskManagerItem::demandsAttentionWinIDs() const
{
    return d->m_demandsAttentionWinIDs;
}

void TaskManagerItem::iconChanged(const QString &windowId)
{
    if(d->m_winIDs.contains(windowId)) {
        d->m_widToIcon.insert(windowId, WindowManager::windowIcon(windowId).isNull() ? QVariant("application-x-desktop") : WindowManager::windowIcon(windowId));
        Q_EMIT dataUpdated(QVector<int>{LingmoUITaskManager::WindowIcons});
        if(d->m_icon.isNull() && windowId == winIDs().first()) {
            d->m_icon = WindowManager::windowIcon(winIDs().at(0));
            Q_EMIT dataUpdated(QVector<int>{LingmoUITaskManager::Icon});
        }
    }
}

void TaskManagerItem::titleChanged(const QString &windowId)
{
    if(d->m_winIDs.contains(windowId)) {
        d->m_widToTitle.insert(windowId, WindowManager::windowTitle(windowId));
        Q_EMIT dataUpdated(QVector<int>{LingmoUITaskManager::WindowTitles});
        if (d->m_name.isEmpty() && windowId == winIDs().first()) {
            d->m_name = WindowManager::windowTitle(winIDs().at(0));
            Q_EMIT dataUpdated(QVector<int>{LingmoUITaskManager::Name});
        }
    }
}

void TaskManagerItem::desktopChanged(const QString &windowId)
{
    if(currentDesktopWinIDs().contains(windowId)) {
        if(!WindowManager::isOnAllDesktops(windowId) && !WindowManager::isOnCurrentDesktop(windowId)) {
            d->m_winIDsOnCurrentDesktop.removeAll(windowId);
            Q_EMIT dataUpdated(QVector<int>{LingmoUITaskManager::CurrentDesktopWinIdList, LingmoUITaskManager::CurrentWinIdList});
        }
    } else if (d->m_winIDs.contains(windowId)){
        if(WindowManager::isOnAllDesktops(windowId) || WindowManager::isOnCurrentDesktop(windowId)) {
            d->m_winIDsOnCurrentDesktop.append(windowId);
            Q_EMIT dataUpdated(QVector<int>{LingmoUITaskManager::CurrentDesktopWinIdList, LingmoUITaskManager::CurrentWinIdList});
        }
    }
}

void TaskManagerItem::demandsAttentionChanged(const QString &windowId)
{
    if(d->m_winIDs.contains(windowId)) {
        if(WindowManager::isDemandsAttention(windowId)) {
            if(!d->m_demandsAttentionWinIDs.contains(windowId)) {
                d->m_demandsAttentionWinIDs.append(windowId);
                Q_EMIT dataUpdated(QVector<int>{LingmoUITaskManager::DemandsAttentionWinIdList});
            }
        } else {
            if(d->m_demandsAttentionWinIDs.contains(windowId)) {
                d->m_demandsAttentionWinIDs.removeAll(windowId);
                Q_EMIT dataUpdated(QVector<int>{LingmoUITaskManager::DemandsAttentionWinIdList});
            }
        }
    }
}

void TaskManagerItem::geometryChanged(const QString &windowId)
{
    if(!d->m_winIDs.contains(windowId)) {
        return;
    }
    const QScreen *oldScreen = d->m_widToScreen.value(windowId);
    if(oldScreen) {
        if(oldScreen->geometry().contains(WindowManager::geometry(windowId).center())) {
            return;
        }
    }
    for(QScreen *screen : QGuiApplication::screens()) {
        if(screen->geometry().contains(WindowManager::geometry(windowId).center())) {
            d->m_widToScreen.insert(windowId, screen);
            break;
        }
    }
    Q_EMIT dataUpdated(QVector<int>{LingmoUITaskManager::WinIdsOnScreens, LingmoUITaskManager::CurrentWinIdList});
}

void TaskManagerItem::refreshWinIdsOnCurrentDesktop()
{
    d->m_winIDsOnCurrentDesktop.clear();
    for(const QString &wid : d->m_winIDs) {
        if(WindowManager::isOnCurrentDesktop(wid) || WindowManager::isOnAllDesktops(wid)) {
            d->m_winIDsOnCurrentDesktop.append(wid);
        }
    }
    Q_EMIT dataUpdated(QVector<int>{LingmoUITaskManager::CurrentDesktopWinIdList, LingmoUITaskManager::CurrentWinIdList});
}

void TaskManagerItem::refreshWinIdsOnScreens()
{
    d->m_widToScreen.clear();
    auto winIds = d->m_winIDs;
    for (QScreen *screen : QGuiApplication::screens()) {
        for(const QString &wid : winIds) {
            if(screen->geometry().contains(WindowManager::geometry(wid).center())) {
                d->m_widToScreen.insert(wid, screen);
                winIds.removeAll(wid);
                break;
            }
        }
    }
    Q_EMIT dataUpdated(QVector<int>{LingmoUITaskManager::WinIdsOnScreens, LingmoUITaskManager::CurrentWinIdList});
}

void TaskManagerItem::refreshWindowTitlesAndIcons()
{
    d->m_widToTitle.clear();
    for(const QString &wid : d->m_winIDs) {
        QString title = WindowManager::windowTitle(wid);
        if(title.isEmpty()) {
            title = d->m_name;
        }
        d->m_widToTitle.insert(wid, title);
        QIcon icon = WindowManager::windowIcon(wid);
        if(icon.isNull()) {
            icon = d->m_icon;
        }
        d->m_widToIcon.insert(wid, icon.isNull() ? QVariant("application-x-desktop") : icon);
    }
}

const QStringList &TaskManagerItem::currentDesktopWinIDs()
{
    return d->m_winIDsOnCurrentDesktop;
}

void TaskManagerItem::updateUnreadMessagesNum(const QString &desktopFile, uint num)
{
    if(desktopFile == d->m_ID) {
        qDebug() << "Update unread messagesNum" << desktopFile << num;
        d->m_unreadMessagesNum = num;
        Q_EMIT dataUpdated(QVector<int>{LingmoUITaskManager::UnreadMessagesNum});
    }
}

uint TaskManagerItem::unReadMessagesNum()
{
    return d->m_unreadMessagesNum;
}

void TaskManagerItem::newInstanceActionActive()
{
    if(d->m_ID.isEmpty()) {
        return;
    }
    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("com.lingmo.ProcessManager"),
                                                          QStringLiteral("/com/lingmo/ProcessManager/AppLauncher"),
                                                          QStringLiteral("com.lingmo.ProcessManager.AppLauncher"),
                                                          QStringLiteral("LaunchApp"));
    message << d->m_ID;

    auto watcher = new QDBusPendingCallWatcher(QDBusPendingCall(QDBusConnection::sessionBus().asyncCall(message)));
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this, [&] (QDBusPendingCallWatcher *self) {
        if (self->isError()) {
            d->m_xdf.startDetached();
        }
        self->deleteLater();
    });

}

void TaskManagerItem::desktopActionActive(const QString &name)
{
    d->m_xdf.actionActivate(name, {});
}

void TaskManagerItem::removeQuickLauncherActionActive()
{
    LingmoUITaskManager::self().setQuickLauncher(d->m_ID, -1);
}

void TaskManagerItem::addQuickLauncherActionActive(int index)
{
    LingmoUITaskManager::self().setQuickLauncher(d->m_ID, index);
}

void TaskManagerItem::exitActionActive(const QStringList &winIds)
{
    for(const QString &wid : winIds) {
        WindowManager::closeWindow(wid);
    }
}

const QMap<QString, QScreen *> &TaskManagerItem::winIdOnScreens() const
{
    return d->m_widToScreen;
}

const QMap<QString, QVariant> &TaskManagerItem::windowTitles() const
{
    return d->m_widToTitle;
}

const QMap<QString, QVariant> &TaskManagerItem::windowIcons() const
{
    return d->m_widToIcon;
}

void TaskManagerItem::updateIconStatus()
{
    Q_EMIT dataUpdated(QVector<int>{LingmoUITaskManager::HasActiveWindow, LingmoUITaskManager::WinIdsOnScreens, LingmoUITaskManager::CurrentWinIdList});
}
