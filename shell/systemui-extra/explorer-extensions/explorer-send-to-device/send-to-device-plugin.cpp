/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2019, Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Ding Jing <dingjing@kylinos.cn>
 *
 */

#include "send-to-device-plugin.h"
#include "file-info.h"

#include <QUrl>
#include <QFile>
#include <QMenu>
#include <QDebug>
#include <QtGlobal>
#include <QProcess>
#include <QTranslator>
#include <QtConcurrent>
#include <QApplication>
#include <file-copy-operation.h>
#include <file-operation-manager.h>
#include <file-info-job.h>
#include <gio/gunixmounts.h>


#ifdef LINGMO_COMMON
#include <lingmosdk/lingmo-com4cxx.h>
#endif

#define V10_SP1_EDU "V10SP1-edu"

static QString getIconName (GIcon* icons);
static void mounted_func (gpointer data, gpointer udata);
static void handle_mount_added  (GVolumeMonitor* monitor, GMount* mount, gpointer data);
static void handle_mount_removed(GVolumeMonitor* monitor, GMount* mount, gpointer data);

using namespace Peony;

SendToPlugin::SendToPlugin(QObject *parent) : QObject(parent), mEnable(true)
{
    // translator
    QTranslator *t = new QTranslator(this);
    t->load(":/translations/explorer-send-to-device_"+QLocale::system().name());
    QFile file(":/translations/explorer-send-to-device_"+QLocale::system().name()+".ts");
    QApplication::installTranslator(t);
}

QList<QAction *> SendToPlugin::menuActions(Types types, const QString &uri, const QStringList &selectionUris)
{
#ifdef LINGMO_COMMON
    if (QString::fromStdString(KDKGetPrjCodeName()) == V10_SP1_EDU) {
        return QList<QAction*>();
    }
#endif

    QList<QAction *> l;

    if (selectionUris.count() <= 0) {
        return l;
    }

    auto info = FileInfo::fromUri(selectionUris.first());

    if (selectionUris.first().startsWith("computer:///")
            || selectionUris.first().startsWith("trash:///")
            || selectionUris.first().startsWith("filesafe:///")
            || info->isVirtual()) {
        return l;
    }

    QStringList selectionTargerUris = selectionUris;
    if (selectionUris.first().startsWith("recent:///")) {
        selectionTargerUris.clear();
        for (auto u : selectionUris){
            selectionTargerUris << FileUtils::getTargetUri(u);
        }
    }

    QAction* action = new DriverAction(selectionTargerUris);
    if (action) {
        l << action;
    }

    Q_UNUSED(uri)
    Q_UNUSED(types)

    return l;
}

DriverItem::DriverItem(QString uri, QIcon icon, QString name, QObject* parent) : QAction(parent), mName(name), mIcon(icon), mUri(uri)
{
    setIcon(mIcon);
    setText(name);
}

const QIcon DriverItem::icon()
{
    return mIcon;
}

const QString DriverItem::name()
{
    return mName;
}

const QString DriverItem::uri()
{
    return mUri;
}

static void handle_mount_added(GVolumeMonitor* monitor, GMount* mount, gpointer data)
{
    DriverAction* drivers = (DriverAction*)data;

    mounted_func (mount, drivers);

    Q_UNUSED(monitor)
}

static void handle_mount_removed(GVolumeMonitor* monitor, GMount* mount, gpointer data)
{
    char*               path = nullptr;
    GFile*              location = nullptr;

    if (!mount || !data) return;

    location = g_mount_get_default_location(mount);
    if (location) {
        path = g_file_get_uri(location);
    }

    if (nullptr != path) {
        ((DriverAction*)data)->driverRemove(path);
    }

    qDebug() << "remove uri:" << path;

    if (!path) g_free(path);
    if (!location) g_object_unref(location);

    Q_UNUSED(monitor)
}

static void mounted_func (gpointer data, gpointer udata)
{
    GMount*             mount = (GMount*)data;
    GFile*              location = nullptr;
    char*               uri = nullptr;

    DriverAction* act = (static_cast<DriverAction*>(udata));

    if (!data || !udata || !act) return;

    if (mount) {
        location = g_mount_get_default_location(mount);
        if (location) {
            uri = g_file_get_uri(location);
            auto path = g_file_peek_path(location);
            if (path) {
                g_autoptr (GUnixMountEntry) entry = g_unix_mount_at(path, NULL);
                if (entry) {
                    auto fsType = g_unix_mount_get_fs_type(entry);
                    if (QString(fsType).contains("fuse.kyfs")) {
                        return;
                    }
                }
            }
        }
    }

    // check permission
    FileInfoJob* fileInfo = new FileInfoJob(uri);
    fileInfo->setAutoDelete ();
    fileInfo->queryAsync ();

    act->connect(fileInfo, &FileInfoJob::queryAsyncFinished, act, [=] (bool success) {
        if (success) {
            GMount* mount = (GMount*)data;
            if (G_IS_MOUNT (mount)) {
                g_autofree char* name = g_mount_get_name(mount);
                GIcon* icons = g_mount_get_icon(mount);
                QString icon = getIconName (icons);

                std::shared_ptr<FileInfo> info = fileInfo->getInfo();
                if (uri && name && info.get()->canExecute() && info.get()->canWrite()) {
                    Q_EMIT (static_cast<DriverAction*>(udata))->driverAdded(uri, name, icon);
                }

                qDebug() << "name:" << name << " uri:" << uri << " icons:" << icons << " icon:" << icon;
            }
        }
    });

    if (!uri) g_free(uri);
    if (!location) g_object_unref(location);
}

static QString getIconName (GIcon* icons)
{
    if (nullptr == icons) {
        return "";
    }

    QString icon = nullptr;

    if (G_IS_ICON(icons)) {
        const gchar* const* iconNames = g_themed_icon_get_names(G_THEMED_ICON (icons));
        if (iconNames) {
            auto p = iconNames;
            while (*p) {
                QIcon icont = QIcon::fromTheme(*p);
                if (!icont.isNull()) {
                    icon = QString (*p);
                    break;
                } else {
                    p++;
                }
            }
        }
        g_object_unref(icons);
    }

    return icon;
}

DriverAction::DriverAction(const QStringList& uris, QObject *parent) : QAction(parent)
{
    mMenu = new QMenu();
    mVolumeMonitor = g_volume_monitor_get();

    mDeviceAdd = g_signal_connect(G_OBJECT(mVolumeMonitor), "mount-added", G_CALLBACK(handle_mount_added), (gpointer)this);
    mDeviceRemove = g_signal_connect(G_OBJECT(mVolumeMonitor), "mount-removed", G_CALLBACK(handle_mount_removed), (gpointer)this);

    connect(this, &DriverAction::driverAdded, this, [=] (QString uri, QString name, QString icon) {
        if (!mDrivers.contains(uri)) {
            QString curUri = uri;
            if (curUri.endsWith("/")) {
                curUri.chop(1);
            }

            QString usrName = QStandardPaths::writableLocation(QStandardPaths::HomeLocation).section("/", -1, -1);

            if ("file:///data" == curUri
                    || "file:///backup" == curUri
                    || "file:///boot" == curUri
                    || "file:///tmp" == curUri
                    || "file:///var" == curUri
                    || "file:///media/" + usrName + "/SYSBOOT" == curUri
                    || "file:///media/" + usrName + "/sysboot" == curUri
                    || "file:///media/" + usrName + "/data" == curUri
                    || "file:///media/" + usrName + "/DATA" == curUri
                    || curUri.startsWith("burn://")
                    || curUri.startsWith("ftp://")
                    || curUri.startsWith("smb://")
                    || curUri.startsWith("file:///media/" + usrName + "/LINGMO-")
                    || curUri.startsWith("file:///media/" + usrName + "/lingmo-")
                    ) {
                return ;
            }

            auto it = new DriverItem (uri, QIcon::fromTheme(icon), name);
            it->connect(it, &QAction::triggered, it, [=] () {
                qDebug() << "======" << __func__ << "uris:" << uris << "it->uri" << it->uri();
                FileCopyOperation* op = new FileCopyOperation(uris, it->uri(), nullptr);
                op->setAutoDelete(true);
                FileOperationManager::getInstance()->startOperation(op);
            });
            mMenu->addAction(it);
            mDrivers[uri] = it;
        }
        showAction();
    });

    connect(this, &DriverAction::driverRemove, this, [=] (QString uri) {
        if (mDrivers.contains(uri)) {
            auto it = mDrivers[uri];
            mMenu->removeAction(it);
            it->deleteLater();
            mDrivers.remove(uri);
        }
        showAction();
    });

    GList* mounts = g_volume_monitor_get_mounts (mVolumeMonitor);
    if (mounts) {
        g_list_foreach (mounts, mounted_func, this);
        g_list_free_full(mounts, g_object_unref);
    }

    setMenu(mMenu);
    setText(tr("Send to a removable device"));
    showAction();
}

DriverAction::~DriverAction()
{
    //if (mMenu) delete mMenu;
    if (mVolumeMonitor) {
        g_signal_handler_disconnect(G_OBJECT(mVolumeMonitor), mDeviceAdd);
        g_signal_handler_disconnect(G_OBJECT(mVolumeMonitor), mDeviceRemove);
        g_object_unref(mVolumeMonitor);
    }
    for (auto it = mDrivers.begin(); it != mDrivers.end(); ++it) {
        it.value()->deleteLater();
    }
    if (!mDrivers.isEmpty()) {
        mDrivers.clear();
    }
}

void DriverAction::showAction()
{
    setVisible(mDrivers.size() > 0 ? true : false);
}
