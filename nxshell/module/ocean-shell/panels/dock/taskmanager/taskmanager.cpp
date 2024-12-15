// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appitem.h"

#include "abstractwindow.h"
#include "abstractwindowmonitor.h"
#include "desktopfileamparser.h"
#include "desktopfileparserfactory.h"
#include "dsglobal.h"
#include "globals.h"
#include "itemmodel.h"
#include "pluginfactory.h"
#include "rolecombinemodel.h"
#include "taskmanager.h"
#include "taskmanageradaptor.h"
#include "taskmanagersettings.h"
#include "treelandwindowmonitor.h"

#include <QGuiApplication>
#include <QStringLiteral>

#include <appletbridge.h>

#ifdef BUILD_WITH_X11
#include "x11windowmonitor.h"
#endif

Q_LOGGING_CATEGORY(taskManagerLog, "ocean.shell.dock.taskmanager", QtInfoMsg)

#define Settings TaskManagerSettings::instance()

#define DESKTOPFILEFACTORY DesktopfileParserFactory<    \
                            DesktopFileAMParser,        \
                            DesktopfileAbstractParser   \
                        >

namespace dock {

TaskManager::TaskManager(QObject* parent)
    : DContainment(parent)
    , m_windowFullscreen(false)
{
    qRegisterMetaType<ObjectInterfaceMap>();
    qDBusRegisterMetaType<ObjectInterfaceMap>();
    qRegisterMetaType<ObjectMap>();
    qDBusRegisterMetaType<ObjectMap>();
    qDBusRegisterMetaType<QStringMap>();
    qRegisterMetaType<QStringMap>();
    qRegisterMetaType<PropMap>();
    qDBusRegisterMetaType<PropMap>();
    qDBusRegisterMetaType<QDBusObjectPath>();

    connect(Settings, &TaskManagerSettings::allowedForceQuitChanged, this, &TaskManager::allowedForceQuitChanged);
    connect(Settings, &TaskManagerSettings::windowSplitChanged, this, &TaskManager::windowSplitChanged);
}

bool TaskManager::load()
{
    loadDockedAppItems();
    auto platformName = QGuiApplication::platformName();
    if (QStringLiteral("wayland") == platformName) {
        m_windowMonitor.reset(new TreeLandWindowMonitor());
    }

#ifdef BUILD_WITH_X11
    else if (QStringLiteral("xcb") == platformName) {
        m_windowMonitor.reset(new X11WindowMonitor());
    }
#endif

    connect(m_windowMonitor.get(), &AbstractWindowMonitor::windowAoceand, this, &TaskManager::handleWindowAoceand);
    return true;
}

bool TaskManager::init()
{
    auto adaptor = new TaskManagerAdaptor(this);
    QDBusConnection::sessionBus().registerService("org.lingmo.ds.Dock.TaskManager");
    QDBusConnection::sessionBus().registerObject("/org/lingmo/ds/Dock/TaskManager", "org.lingmo.ds.Dock.TaskManager", this);

    DApplet::init();

    DS_NAMESPACE::DAppletBridge bridge("org.lingmo.ds.ocean-apps");
    if (auto applet = bridge.applet()) {
        auto model = applet->property("appModel").value<QAbstractItemModel *>();
        Q_ASSERT(model);
        m_activeAppModel =
            new RoleCombineModel(m_windowMonitor.data(), model, AbstractWindow::identityRole, [](QVariant data, QAbstractItemModel *model) -> QModelIndex {
                auto roleNames = model->roleNames();
                QList<QByteArray> identifiedOrders = {"desktopId", "startupWMClass", "name", "iconName"};

                auto indentifies = data.toStringList();
                for (auto id : indentifies) {
                    if (id.isEmpty()) {
                        continue;
                    }

                    for (auto identifiedOrder : identifiedOrders) {
                        auto res = model->match(model->index(0, 0), roleNames.key(identifiedOrder), id, 1, Qt::MatchFixedString | Qt::MatchWrap);
                        if (res.size() > 0 && res.first().isValid()) {
                            return res.first();
                        }
                    }
                }

                return QModelIndex();
            });
    }

    if (m_windowMonitor)
        m_windowMonitor->start();

    connect(m_windowMonitor.data(), &AbstractWindowMonitor::windowFullscreenChanged, this, [this] (bool isFullscreen) {
        m_windowFullscreen = isFullscreen;
        emit windowFullscreenChanged(isFullscreen);
    });
    return true;
}

ItemModel* TaskManager::dataModel()
{
    return ItemModel::instance();
}

void TaskManager::handleWindowAoceand(QPointer<AbstractWindow> window)
{
    if (!window || window->shouldSkip() || window->getAppItem() != nullptr) return;

    // TODO: remove below code and use use model replaced.
    QModelIndexList res;
    if (m_activeAppModel) {
        res = m_activeAppModel->match(m_activeAppModel->index(0, 0), AbstractWindow::winIdRole, window->id());
    }

    QSharedPointer<DesktopfileAbstractParser> desktopfile = nullptr;
    QString desktopId;
    if (res.size() > 0) {
        desktopId = res.first().data(m_activeAppModel->roleNames().key("desktopId")).toString();
    }

    if (!desktopId.isEmpty()) {
        desktopfile = DESKTOPFILEFACTORY::createById(desktopId, "amAPP");
    }

    if (desktopfile.isNull() || !desktopfile->isValied().first) {
        desktopfile = DESKTOPFILEFACTORY::createByWindow(window);
    }

    auto appitem = desktopfile->getAppItem();

    if (appitem.isNull() || (appitem->hasWindow() && windowSplit())) {
        auto id = windowSplit() ? QString("%1@%2").arg(desktopfile->id()).arg(window->id()) : desktopfile->id();
        appitem = new AppItem(id);
    }

    appitem->appendWindow(window);
    appitem->setDesktopFileParser(desktopfile);

    ItemModel::instance()->addItem(appitem);
}

void TaskManager::clickItem(const QString& itemId, const QString& menuId)
{
    auto item = ItemModel::instance()->getItemById(itemId);
    if(!item) return;

    if (menuId == DOCK_ACTION_ALLWINDOW) {
        QList<uint32_t> windowIds;
        auto windows = item->data().toStringList();
        std::transform(windows.begin(), windows.end(), std::back_inserter(windowIds), [](const QString &windowId) {
            return windowId.toUInt();
        });

        m_windowMonitor->presentWindows(windowIds);
        return;
    }

    item->handleClick(menuId);
}

void TaskManager::showItemPreview(const QString &itemId, QObject* relativePositionItem, int32_t previewXoffset, int32_t previewYoffset, uint32_t direction)
{
    auto item = ItemModel::instance()->getItemById(itemId).get();
    if (!item) return;

    QPointer<AppItem> pItem = reinterpret_cast<AppItem*>(item);
    if (pItem.isNull()) return;

    m_windowMonitor->showItemPreview(pItem, relativePositionItem, previewXoffset, previewYoffset, direction);
}

void TaskManager::hideItemPreview()
{
    m_windowMonitor->hideItemPreview();
}

void TaskManager::setAppItemWindowIconGeometry(const QString& appid, QObject* relativePositionItem, const int& x1, const int& y1, const int& x2, const int& y2)
{
    QPointer<AppItem> item = static_cast<AppItem*>(ItemModel::instance()->getItemById(appid).get());
    if (item.isNull()) return;

    for (auto window : item->getAppendWindows()) {
        window->setWindowIconGeometry(qobject_cast<QWindow*>(relativePositionItem), QRect(QPoint(x1, y1),QPoint(x2, y2)));
    }
}

void TaskManager::loadDockedAppItems()
{
    // TODO: add support for group and dir type
    for (const auto& appValueRef : TaskManagerSettings::instance()->dockedDesktopFiles()) {
        auto app = appValueRef.toObject();
        auto appid = app.value("id").toString();
        auto type = app.value("type").toString();
        auto desktopfile = DESKTOPFILEFACTORY::createById(appid, type);
        auto valid = desktopfile->isValied();

        if (!valid.first) {
            qInfo(taskManagerLog()) << "failed to load " << appid << " beacause " << valid.second;
            continue;
        }

        auto appitem = desktopfile->getAppItem();
        if (appitem.isNull()) {
            appitem = new AppItem(appid);
        }

        appitem->setDesktopFileParser(desktopfile);
        ItemModel::instance()->addItem(appitem);
    }
}

bool TaskManager::allowForceQuit()
{
    return Settings->isAllowedForceQuit();
}

QString TaskManager::desktopIdToAppId(const QString& desktopId)
{
    return Q_LIKELY(desktopId.endsWith(".desktop")) ? desktopId.chopped(8) : desktopId;
}

bool TaskManager::requestDockByDesktopId(const QString& desktopID)
{
    if (desktopID.startsWith("internal/")) return false;
    return RequestDock(desktopIdToAppId(desktopID));
}

bool TaskManager::requestUndockByDesktopId(const QString& desktopID)
{
    if (desktopID.startsWith("internal/")) return false;
    return RequestUndock(desktopIdToAppId(desktopID));
}

bool TaskManager::RequestDock(QString appID)
{
    auto desktopfileParser = DESKTOPFILEFACTORY::createById(appID, "amAPP");

    auto res = desktopfileParser->isValied();
    if (!res.first) {
        qCWarning(taskManagerLog) << res.second;
        return false;
    }

    QPointer<AppItem> appitem = desktopfileParser->getAppItem();
    if (appitem.isNull()) {
        appitem = new AppItem(appID);
        appitem->setDesktopFileParser(desktopfileParser);
        ItemModel::instance()->addItem(appitem);
    }
    appitem->setDocked(true);
    return true;
}

bool TaskManager::IsDocked(QString appID)
{
    auto desktopfileParser = DESKTOPFILEFACTORY::createById(appID, "amAPP");

    auto res = desktopfileParser->isValied();
    if (!res.first) {
        qCWarning(taskManagerLog) << res.second;
        return false;
    }

    QPointer<AppItem> appitem = desktopfileParser->getAppItem();
    if (appitem.isNull()) {
        return false;
    }
    return appitem->isDocked();
}

bool TaskManager::RequestUndock(QString appID)
{
    auto desktopfileParser = DESKTOPFILEFACTORY::createById(appID, "amAPP");
    auto res = desktopfileParser->isValied();
    if (!res.first) {
        qCWarning(taskManagerLog) << res.second;
        return false;
    }
    QPointer<AppItem> appitem = desktopfileParser->getAppItem();
    if (appitem.isNull()) {
        return false;
    }
    appitem->setDocked(false);
    return true;
}

bool TaskManager::windowSplit()
{
    return Settings->isWindowSplit();
}

bool TaskManager::windowFullscreen()
{
    return m_windowFullscreen;
}

D_APPLET_CLASS(TaskManager)
}

#include "taskmanager.moc"
