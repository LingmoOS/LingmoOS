/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "intel-side-bar-menu.h"
#include "intel-side-bar-abstract-item.h"

#include "bookmark-manager.h"
#include "properties-window.h"

#include "menu-plugin-manager.h"

#include "file-utils.h"
#include "file-info.h"
#include "file-info-job.h"

#include <QAction>
#include <QModelIndex>
#include <QProcess>
#include <format_dialog.h>

#include <QDebug>
#include <volume-manager.h>

using namespace Peony::Intel;

SideBarMenu::SideBarMenu(SideBarAbstractItem *item, SideBar *sideBar, QWidget *parent) : QMenu (parent)
{
    m_uri = item->uri();
    m_item = item;
    m_side_bar = sideBar;

    if (!item) {
        auto action = addAction(QIcon::fromTheme("preview-file"), tr("Properties"));
        action->setEnabled(false);
        return;
    }

    switch (item->type()) {
    case SideBarAbstractItem::FavoriteItem: {
        constructFavoriteActions();
        break;
    }
    case SideBarAbstractItem::PersonalItem: {
        constructPersonalActions();
        break;
    }
    case SideBarAbstractItem::FileSystemItem: {
        constructFileSystemItemActions();
        break;
    }
    default: {
        auto action = addAction(QIcon::fromTheme("preview-file"), tr("Properties"));
        action->setEnabled(false);
        break;
    }
    }
}

const QList<QAction *> SideBarMenu::constructFavoriteActions()
{
    QList<QAction *> l;

    l<<addAction(QIcon::fromTheme("window-close-symbolic"), tr("Delete Symbolic"), [=]() {
        BookMarkManager::getInstance()->removeBookMark(m_uri);
    });

    if (!m_item->firstColumnIndex().parent().isValid()) {
        l.last()->setEnabled(false);
    } else if (m_item->firstColumnIndex().row() < 10) {
        l.last()->setEnabled(false);
    }
    else if (m_uri == "file:///data/usershare" || m_uri == "kmre:///" || m_uri == "kydroid:///")
    {
        //fix bug#68431, can not delete option issue
        l.last()->setEnabled(false);
    }

    l<<addAction(QIcon::fromTheme("preview-file"), tr("Properties"), [=]() {
        PropertiesWindow *w = new PropertiesWindow(QStringList()<<m_uri);
        w->show();
    });
    if (!m_item->firstColumnIndex().parent().isValid()) {
        l.last()->setEnabled(false);
    }

    return l;
}

const QList<QAction *> SideBarMenu::constructPersonalActions()
{
    QList<QAction *> l;

    l<<addAction(QIcon::fromTheme("preview-file"), tr("Properties"), [=]() {
        PropertiesWindow *w = new PropertiesWindow(QStringList()<<m_uri);
        w->show();
    });

    return l;
}

const QList<QAction *> SideBarMenu::constructFileSystemItemActions()
{
    QList<QAction *> l;

    //FIXME: replace BLOCKING api in ui thread.
    auto info = FileInfo::fromUri(m_uri);
    if (info->displayName().isEmpty()) {
        FileInfoJob j(info);
        j.querySync();
    }

    if (info->canUnmount() || info->canMount()) {
        l<<addAction(QIcon::fromTheme("media-eject-symbolic"), tr("Unmount"), [=]() {
            m_item->unmount();
        });
        bool isUmountable = FileUtils::isFileUnmountable(m_item->uri());
        bool isMounted = isUmountable;
        auto targetUri = FileUtils::getTargetUri(m_item->uri());
        if (!targetUri.isEmpty()) {
            if (targetUri == "burn:///") {
                isMounted = false;
            } else {
                isMounted = (targetUri != "file:///") || isUmountable;
            }
        }

        l.last()->setEnabled(isMounted);
    }
    if(0 != QString::compare(m_uri, "computer:///root.link"))
    {
        if (m_item->isRemoveable()) {
            l<<addAction(QIcon::fromTheme("media-eject-symbolic"), tr("Eject"), [=](){
                m_item->eject(G_MOUNT_UNMOUNT_NONE);
            });

            l.last()->setEnabled(m_item->isMounted());
        }
    }

    if(0 != QString::compare(m_uri, "filesafe:///")) {
        auto mgr = MenuPluginManager::getInstance();
        auto ids = mgr->getPluginIds();
        for (auto id : ids) {
            auto factory = mgr->getPlugin(id);
            //qDebug()<<id;
            auto tmp = factory->menuActions(MenuPluginInterface::SideBar, m_uri, QStringList()<<m_uri);
            addActions(tmp);
            for (auto action : tmp) {
                action->setParent(this);
            }
            l<<tmp;
        }
    }

    l<<addAction(QIcon::fromTheme("preview-file"), tr("Properties"), [=]() {
        //fix computer show properties crash issue, link to bug#77789
        if (m_uri == "computer:///" || m_uri == "//")
        {
            gotoAboutComputer();
        }
        else{
            PropertiesWindow *w = new PropertiesWindow(QStringList()<<m_uri);
            w->show();
        }
    });
    if ((0 != QString::compare(m_uri, "computer:///")) &&
        (0 != QString::compare(m_uri, "filesafe:///"))) {
        l.last()->setEnabled(m_item->isMounted());
    }

    /*
     *  add format function
     *  provide option for all mountable device
     *  if can not format, will have prompt
     */
    auto targetUri = FileUtils::getTargetUri(m_uri);
    auto mount = VolumeManager::getMountFromUri(targetUri);
    QString unixDevice = FileUtils::getUnixDevice(m_uri);
    //qDebug() << "targetUri:"<<targetUri<<m_uri;
    //fix erasable optical disk can be format issue, bug#32415
    //fix bug#52491, CDROM and DVD can format issue
    if((! unixDevice.isNull() && ! unixDevice.contains("/dev/sr"))
       && info->isVolume() && info->canUnmount()){
          l<<addAction(QIcon::fromTheme("preview-file"), tr("format"), [=]() {
          Format_Dialog *fd  = new Format_Dialog(m_uri, nullptr, nullptr);
          fd->show();
      });
      //no right u-disk should not be formated, fix bug#
      if (! mount)
          l.last()->setEnabled(false);
    }
    return l;
}

void SideBarMenu::gotoAboutComputer()
{
    QProcess p;
    p.setProgram("lingmo-control-center");
    //-m About para to show about computer infos, related to bug#88258
    p.setArguments(QStringList()<<"-m" << "About");
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    p.startDetached();
#else
    p.startDetached("lingmo-control-center", QStringList()<<"-m" << "About");
#endif
    p.waitForFinished(-1);
}

const QList<QAction *> SideBarMenu::constructNetWorkItemActions()
{
    QList<QAction *> l;

    /* 共享文件夹无右键菜单'卸载' */
    if (!m_uri.startsWith("file://")) {
        l<<addAction(QIcon::fromTheme("media-eject-symbolic"), tr("Unmount"), [=]() {
            m_item->unmount();
        });       
        l.last()->setEnabled(m_item->isMounted());
    }

    //fix network items has Properties option issue, link to bug#78617
    if (! m_uri.startsWith("network://")) {
        l<<addAction(QIcon::fromTheme("preview-file"), tr("Properties"), [=]() {
            PropertiesWindow *w = new PropertiesWindow(QStringList()<<m_uri);
            w->show();
        });
    }

    return l;

}

