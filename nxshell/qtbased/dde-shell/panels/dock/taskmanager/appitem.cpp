// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "globals.h"
#include "appitem.h"
#include "abstractitem.h"
#include "abstractwindow.h"
#include "desktopfileabstractparser.h"
#include "taskmanagersettings.h"

#include <QPointer>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QStringLiteral>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(appitemLog, "dde.shell.dock.taskmanger.appitem")

namespace dock {
AppItem::AppItem(QString id, QObject *parent)
    : AbstractItem(QStringLiteral("AppItem/%1").arg(escapeToObjectPath(id)), parent)
    , m_id(id)
{
    connect(this, &AbstractItem::dockedChanged, this, &AppItem::checkAppItemNeedDeleteAndDelete);
    connect(this, &AbstractItem::dataChanged, this, &AppItem::checkAppItemNeedDeleteAndDelete);

    connect(this, &AppItem::currentActiveWindowChanged, this, &AbstractItem::iconChanged);
}

AppItem::~AppItem()
{
    qCDebug(appitemLog()) << "destory appitem: " << m_id;
}

AbstractItem::ItemType AppItem::itemType() const
{
    return AbstractItem::AppType;
}

QString AppItem::id() const
{
    return m_id;
}

QString AppItem::type() const
{
    return m_desktopfileParser && !m_desktopfileParser.isNull() ?
        m_desktopfileParser->type() : "none";
}

QString AppItem::icon() const
{
    if (m_currentActiveWindow.isNull() || (m_desktopfileParser && m_desktopfileParser->isValied().first))
        return m_desktopfileParser ? m_desktopfileParser->desktopIcon() : "application-default-icon";
    else {
        return m_currentActiveWindow->icon();
    }

    // QString icon;
    // if (m_currentActiveWindow) {
    //     icon = m_currentActiveWindow->icon();
    // }
    // if (icon.isEmpty() && m_desktopfileParser && !m_desktopfileParser.isNull()) {
    //     icon = m_desktopfileParser->desktopIcon();
    // }
    // return icon;
}

QString AppItem::name() const
{
    if (m_desktopfileParser && !m_desktopfileParser.isNull())
        return (m_desktopfileParser->xDeepinVendor() == QStringLiteral("deepin")) ? m_desktopfileParser->genericName() : m_desktopfileParser->name();
    return "";
}

QString AppItem::menus() const
{
    bool isDesltopfileParserAvaliable = m_desktopfileParser && !m_desktopfileParser.isNull() && m_desktopfileParser->isValied().first;
    QJsonArray array;
    QJsonObject launchMenu;

    launchMenu["id"] = DOCK_ACTIN_LAUNCH;
    launchMenu["name"] = hasWindow() ?
                            isDesltopfileParserAvaliable ? name() : m_windows.first()->title()
                        :tr("Open");

    array.append(launchMenu);

    if (isDesltopfileParserAvaliable) {
        for (auto& [id, name] : m_desktopfileParser->actions()) {
            QJsonObject object;
            object["id"] = id;
            object["name"] = name;
            array.append(object);
        }
    }

    // Temporarily disable all windows action for the related functionality missing in deepin-kwin
    // if (hasWindow()) {
    //     QJsonObject allWindowMenu;
    //     allWindowMenu["id"] = DOCK_ACTION_ALLWINDOW;
    //     allWindowMenu["name"] = tr("All Windows");
    //     array.append(allWindowMenu);
    // }

    QJsonObject dockMenu;
    dockMenu["id"] = DOCK_ACTION_DOCK;
    dockMenu["name"] = isDocked() ? tr("Undock") : tr("Dock");
    array.append(dockMenu);

    if (hasWindow()) {
        QJsonObject foreceQuit;
        foreceQuit["id"] = DOCK_ACTION_FORCEQUIT;
        foreceQuit["name"] = tr("Force Quit");
        if (TaskManagerSettings::instance()->isAllowedForceQuit()) {
            array.append(foreceQuit);
        }

        QJsonObject closeAll;
        closeAll["id"] = DOCK_ACTION_CLOSEALL;
        closeAll["name"] = tr("Close All");
        array.append(closeAll);
    }

    return QJsonDocument(array).toJson();
}

QString AppItem::desktopfileID() const
{
    if (m_desktopfileParser && !m_desktopfileParser.isNull()) {
        return m_desktopfileParser->id();
    }
    return "";
}

bool AppItem::isActive() const
{
    return m_currentActiveWindow && m_currentActiveWindow->isActive();
}

void AppItem::active() const
{
    if (m_currentActiveWindow) {
        if (!isActive()) {
            m_currentActiveWindow->activate();
        } else if (m_windows.size() == 1) {
            m_currentActiveWindow->minimize();
        } else if (m_windows.size() > 1) {
            auto size = m_windows.size();

            for (int i = 0; i < size; i++) {
                if (m_windows[i] == m_currentActiveWindow) {
                    m_windows[(i+1) % size]->activate();
                    break;
                }
            }
        }
    }
}

bool AppItem::isAttention() const
{
    bool res = false;
    for (const auto window : m_windows) {
        if (window->isAttention()) {
            res = true;
            break;
        }
    }
    return res;
}

bool AppItem::isDocked() const
{
    return m_desktopfileParser &&
            !m_desktopfileParser.isNull() &&
            m_desktopfileParser->isDocked();
}

void AppItem::setDocked(bool docked)
{
    if (docked && docked == isDocked()) return;
    if (m_desktopfileParser && !m_desktopfileParser.isNull()) {
        m_desktopfileParser->setDocked(docked);
    }
}

void AppItem::handleClick(const QString& clickItem)
{
    if (clickItem.isEmpty()) {
        if (m_windows.size() == 0) {
            launch();
        } else {
            active();
        }
    } else {
        handleMenu(clickItem);
    }

}

QVariant AppItem::data()
{
    QStringList ret;
    for (auto window : m_windows) {
        ret.append(QString::number(window->id()));
    }
    return ret;
}

bool AppItem::hasWindow() const
{
    return m_windows.size() > 0;
}

void AppItem::launch()
{
    if (m_desktopfileParser && !m_desktopfileParser.isNull()) {
        m_desktopfileParser->launch();
    }
}

void AppItem::requestQuit()
{
    for (auto window : m_windows) {
        window->killClient();
    }
}

void AppItem::handleMenu(const QString& menuId)
{
    if (menuId == DOCK_ACTIN_LAUNCH) {
        launch();
    } else if (menuId == DOCK_ACTION_DOCK) {
        setDocked(!isDocked());
    } else if (menuId == DOCK_ACTION_FORCEQUIT) {
        requestQuit();
    } else if (menuId == DOCK_ACTION_CLOSEALL) {
        for (auto window : m_windows) {
            window->close();
        }
    } else if (m_desktopfileParser && !m_desktopfileParser.isNull()){
        m_desktopfileParser->launchWithAction(menuId);
    }
}

void AppItem::appendWindow(QPointer<AbstractWindow> window)
{
    m_windows.append(window);
    window->setAppItem(QPointer<AppItem>(this));
    Q_EMIT AbstractItem::dataChanged();
    Q_EMIT appendedWindow(window);

    if (window->isActive() || m_windows.size() == 1) updateCurrentActiveWindow(window);
    connect(window.get(), &QObject::destroyed, this, &AppItem::onWindowDestroyed, Qt::UniqueConnection);
    connect(window.get(), &AbstractWindow::stateChanged, this, [window, this](){
        if(window->isActive()) {
            updateCurrentActiveWindow(window);
        }
        Q_EMIT activeChanged();
        Q_EMIT attentionChanged();
    });
}

void AppItem::setDesktopFileParser(QSharedPointer<DesktopfileAbstractParser> desktopfile)
{
    if (m_desktopfileParser == desktopfile) return;

    if (m_desktopfileParser) disconnect(m_desktopfileParser.get());

    m_desktopfileParser = desktopfile;
    connect(m_desktopfileParser.get(), &DesktopfileAbstractParser::nameChanged, this, &AbstractItem::nameChanged);
    connect(m_desktopfileParser.get(), &DesktopfileAbstractParser::iconChanged, this, &AbstractItem::iconChanged);
    connect(m_desktopfileParser.get(), &DesktopfileAbstractParser::actionsChanged, this, &AbstractItem::menusChanged);
    connect(m_desktopfileParser.get(), &DesktopfileAbstractParser::dockedChanged, this, &AbstractItem::menusChanged);
    connect(m_desktopfileParser.get(), &DesktopfileAbstractParser::dockedChanged, this, &AbstractItem::dockedChanged);
    connect(m_desktopfileParser.get(), &DesktopfileAbstractParser::genericNameChanged, this, &AbstractItem::nameChanged);

    desktopfile->addAppItem(this);
}

QPointer<DesktopfileAbstractParser> AppItem::getDesktopFileParser()
{
    return m_desktopfileParser.get();
}

void AppItem::removeWindow(QPointer<AbstractWindow> window)
{
    m_windows.removeAll(window);
    Q_EMIT AbstractItem::dataChanged();

    if (m_currentActiveWindow.get() == window && m_windows.size() > 0) {
        updateCurrentActiveWindow(m_windows.last().get());
    }
}

const QList<QPointer<AbstractWindow>>& AppItem::getAppendWindows()
{
    return m_windows;
}

QString AppItem::getCurrentActiveWindowName() const
{
    return m_currentActiveWindow.isNull() ? this->name() : m_currentActiveWindow->title();
}

QString AppItem::getCurrentActiveWindowIcon() const
{
    return m_currentActiveWindow.isNull() ? this->icon() : m_currentActiveWindow->icon();
}

void AppItem::updateCurrentActiveWindow(QPointer<AbstractWindow> window)
{
    Q_ASSERT(m_windows.contains(window));

    if (m_currentActiveWindow && !m_currentActiveWindow.isNull()) {
        disconnect(m_currentActiveWindow, &AbstractWindow::iconChanged, this, &AppItem::iconChanged);
    }

    m_currentActiveWindow = window;
    connect(m_currentActiveWindow.get(), &AbstractWindow::iconChanged, this, &AppItem::iconChanged);

    Q_EMIT currentActiveWindowChanged();
}

void AppItem::checkAppItemNeedDeleteAndDelete()
{
    if (hasWindow()) {
        return;
    }

    if (isDocked()) {
        return;
    }

    deleteLater();
}

void AppItem::onWindowDestroyed()
{
    auto window = qobject_cast<AbstractWindow*>(sender());
    removeWindow(window);
}

}
