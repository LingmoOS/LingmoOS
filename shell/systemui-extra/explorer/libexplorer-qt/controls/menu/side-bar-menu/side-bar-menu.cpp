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

#include "side-bar-menu.h"
#include "side-bar-abstract-item.h"
#include "side-bar-file-system-item.h"

#include "bookmark-manager.h"
//#include "properties-window.h"
#include "properties-window-factory-plugin-manager.h"
#include "menu-plugin-manager.h"

#include "file-utils.h"
#include "file-info.h"
#include "file-info-job.h"

#include <QAction>
#include <QModelIndex>
#include "format_dialog.h"
#include "format-dlg-create-delegate.h"

#include <QApplication>
#include <QProcess>

#ifndef LINGMO_UDF_BURN
#include "disccontrol.h"
#include "udfFormatDialog.h"
#else
#include <libkyudfburn/disccontrol.h>
#include "ky-udf-format-dialog.h"
#include "udfAppendBurnDataDialog.h"
using namespace UdfBurn;
#endif

#include <QDebug>
#include <volume-manager.h>

using namespace Peony;

static const int FAVORITE_CAN_NOT_DELETE_URI_COUNT=3;

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
    case SideBarAbstractItem::NetWorkItem: {
        constructNetWorkItemActions();
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

    l<<addAction(QIcon::fromTheme("edit-clear-symbolic"), tr("Delete Symbolic"), this, [=]() {
        BookMarkManager::getInstance()->removeBookMark(m_uri);
    });

    if (!m_item->firstColumnIndex().parent().isValid()) {
        l.last()->setEnabled(false);
    } else if (m_item->firstColumnIndex().row()<FAVORITE_CAN_NOT_DELETE_URI_COUNT) {
        l.last()->setEnabled(false);
    }
    else if (m_uri == "favorite:///data/usershare?schema=file" || m_uri == "kmre:///" || m_uri == "kydroid:///")
    {
        //fix bug#68431, can not delete option issue
        l.last()->setEnabled(false);
    }

    l<<addAction(QIcon::fromTheme("preview-file"), tr("Properties"), this, [=]() {
        QMainWindow *w = PropertiesWindowFactoryPluginManager::getInstance()->create(QStringList()<<m_uri);
        //PropertiesWindow *w = new PropertiesWindow(QStringList()<<m_uri);
        if(this->parentWidget() && this->parentWidget()->isModal()){
            w->setParent(this->parentWidget());
        }
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

    l<<addAction(QIcon::fromTheme("preview-file"), tr("Properties"), this, [=]() {
        QMainWindow *w = PropertiesWindowFactoryPluginManager::getInstance()->create(QStringList()<<m_uri);
        //PropertiesWindow *w = new PropertiesWindow(QStringList()<<m_uri);
        if(this->parentWidget() && this->parentWidget()->isModal()){
            w->setParent(this->parentWidget());
        }
        w->show();
    });

    return l;
}

#include "file-enumerator.h"
const QList<QAction *> SideBarMenu::constructFileSystemItemActions()
{
    QList<QAction *> l;
    /* 卸载 */
    bool isWayland = qApp->property("isWayland").toBool(); // related to #105070
    bool isReddisk = false;
    QString unixDevice = m_item->getDevice();
    QString uri;
    if(m_uri=="file:///") /* 文件系统特殊处理 */
        uri = "computer:///root.link";
    else if(!unixDevice.isEmpty())/* 由于格式化、属性、插件（例如：发送到移动设备）等未重构，还是需要用之前的computer uri */
        uri = getComputerUriFromUnixDevice(unixDevice);
    else
        uri=m_uri;

    if (!unixDevice.isEmpty() && uri.isEmpty()) {
        //可能是加密分区数据未同步问题，尝试同步
        auto fsItem = qobject_cast<SideBarFileSystemItem *>(m_item);
        auto gvolume = fsItem->getVolume().getGVolume();
        g_autofree gchar *unix_device = g_volume_get_identifier(gvolume, G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE);
        unixDevice = unix_device;
        uri = getComputerUriFromUnixDevice(unixDevice);
    }

    //fix bug#212689, 212690, 213120, 213121, hide reddisk format and unmount option
    if (unixDevice.startsWith("/dev/dm") && QFile::exists("/opt/AQTJ/Client/JC/MAIN/bin/jc_main_ui"))
        isReddisk = true;

    //fix bug#175330, wayland should be the same with mainline version
//    if (isWayland) {
//        if (m_item->isUnmountable()) {
//            l<<addAction(QIcon::fromTheme("media-eject-symbolic"), tr("Unmount"), this, [=]() {
//                m_item->unmount();
//            });
//            l.last()->setEnabled(m_item->isMounted());
//        }
//    } else {
    /*  可用的U盘、外接移动硬盘、外接移动光盘, 右键菜单里不允许有“卸载”选项，bug#83206 */
    if (! isReddisk && !(m_item->isEjectable() || m_item->isStopable()) && m_item->isUnmountable()) {
        l<<addAction(QIcon::fromTheme("media-eject-symbolic"), tr("Unmount"), this, [=]() {
            m_item->unmount();
        });
        l.last()->setEnabled(m_item->isMounted());
    }
//    }

    /* 弹出 */
    if (m_item->isEjectable()||m_item->isStopable()) {
        l<<addAction(QIcon::fromTheme("media-eject-symbolic"), tr("Eject"), this, [=](){
            m_item->eject(G_MOUNT_UNMOUNT_NONE);
        });

        if(m_item->getDevice().contains("/dev/sr")){/* 光盘在刻录数据、镜像等操作时,若处于busy状态时，弹出菜单置灰不可用。 */
            l.last()->setDisabled(FileUtils::isBusyDevice(m_item->getDevice()));
        }
    }

    //not allow format data block, fix bug#66471，66479
    QString targetUri = FileUtils::getTargetUri(m_uri);
    bool isData = m_uri == "file:///data" || targetUri == "file:///data"
            || (m_uri.startsWith("file:///media") && m_uri.endsWith("/data"))
            || (targetUri.startsWith("file:///media") && targetUri.endsWith("/data"));

    /* 文件系统、手机要求不能格式化 */
    /* 没有uri的item不能格式化，FormatDialog需要uri走流程，否则会导致崩溃问题 */
    //fix bug#92380, file system has format option issue
    bool showFormatDialog = m_uri!="file:///" && m_uri != "computer:///root.link"
            && (!unixDevice.isNull())
            && !unixDevice.startsWith("/dev/bus/usb")
            && (m_item->isVolume()) && !m_item->uri().isEmpty()
            && ! isReddisk;

    //fix bug133116, not allow format data disk
    if(showFormatDialog && ! isData)
    {
        if(unixDevice.contains("/dev/sr")){/*  光盘格式化(udf格式化) */
            QAction *action = addAction(QIcon::fromTheme("preview-file"), tr("Format"));
            action->setEnabled(false);
            l.append(action);
#ifndef LINGMO_UDF_BURN
            if(!FileUtils::isBusyDevice(m_item->getDevice())){/* 光盘在刻录数据、镜像等操作时,即若处于busy状态时，该菜单置灰不可用。link to bug#143293  */
                DiscControl *discControl = new DiscControl(unixDevice);
                if(discControl->work()){
                   connect(discControl, &DiscControl::workFinished, this, [=](DiscControl *discCtrl){
                       connect(action, &QAction::triggered, this, [=](){
                           UdfFormatDialog *udfFormatDlg = FormatDlgCreateDelegate::getInstance()->createUdfDlg(uri, discCtrl);
                           udfFormatDlg->show();
                       });
                       qDebug()<<unixDevice<<" supported Udf values are:"<<discCtrl->supportUdf();
                       action->setEnabled(discCtrl->supportUdf());
                   });
                }
            }
#else
            if(!FileUtils::isBusyDevice(m_item->getDevice())){/* 光盘在刻录数据、镜像等操作时,即若处于busy状态时，该菜单置灰不可用。link to bug#143293  */
                UdfBurn::DiscControl *discControl = new UdfBurn::DiscControl(unixDevice);
                if(discControl->work()){
                   connect(discControl, &UdfBurn::DiscControl::workFinished, this, [=](UdfBurn::DiscControl *discCtrl){
                       connect(action, &QAction::triggered, this, [=](){
                           UdfBurn::UdfFormatDialogWrapper *udfFormatDlg = FormatDlgCreateDelegate::getInstance()->createUdfDlgWrapper(uri, discCtrl);
                           udfFormatDlg->show();
                       });
                       qDebug()<<unixDevice<<" supported Udf values are:"<<discCtrl->supportUdf();
                       action->setEnabled(discCtrl->supportUdf());
                   });
                }
            }
#endif
        }else{/* 其它格式化 */
            l<<addAction(QIcon::fromTheme("preview-file"), tr("Format"), this, [=]() {
                auto info = FileInfo::fromUri(uri);
                if (info->targetUri ().isEmpty ()) {
                    FileInfoJob job (uri, this);
                    job.querySync ();
                }
                Format_Dialog *fd = FormatDlgCreateDelegate::getInstance()->createUDiskDlg(uri, m_item);
                fd->show();
            });
        }
    }
    /* 插件 */
    if (!isWayland && isData) {
        //skip
    } else {
        if(0 != QString::compare(m_uri, "filesafe:///")) {
            auto mgr = MenuPluginManager::getInstance();
            auto ids = mgr->getPluginIds();
            for (auto id : ids) {
                auto factory = mgr->getPlugin(id);
                //qDebug()<<id;
                auto tmp = factory->menuActions(MenuPluginInterface::SideBar, uri, QStringList()<<uri);
                addActions(tmp);
                for (auto action : tmp) {
                    action->setParent(this);
                }
                l<<tmp;
            }
        }
    }

#ifdef LINGMO_UDF_BURN
    /* udf刻录--R类型光盘 */
    if(unixDevice.contains("/dev/sr")){
        /* 光盘追加刻录 ( udf 追加刻录) */
        qDebug() << "侧边栏： append udf format action.";
        QAction *actionBurn = addAction(QIcon::fromTheme("preview-file"), tr("burndata"));
        actionBurn->setEnabled(false);
        l.append(actionBurn);

        if(!FileUtils::isBusyDevice(m_item->getDevice())) {
            /* 光盘在刻录数据、镜像等操作时,即若处于busy状态时，该菜单置灰不可用。link to bug#143293  */
            DiscControl *discControl = new DiscControl(unixDevice);
            if(discControl->work()){
                connect(discControl, &DiscControl::workFinished, this, [=](DiscControl *discCtrl){
                    connect(actionBurn, &QAction::triggered, this, [=](){
                        UdfAppendBurnDataDialog *udfAppendBurnDataDlg = new UdfAppendBurnDataDialog(uri, discCtrl);
                        udfAppendBurnDataDlg->show();
                    });

                    qDebug() << unixDevice << "侧边栏： supported Udf appendBurnData values : "<<discCtrl->discCanAppend();
                    l.last()->setEnabled(discCtrl->discCanAppend() && discControl->isAllRType());
                });
            }
        }
    }
#endif

    /* 属性 */
    l<<addAction(QIcon::fromTheme("preview-file"), tr("Properties"), this, [=]() {
        //fix computer show properties crash issue, link to bug#77789
        if (m_uri == "computer:///" || m_uri == "//")
        {
            gotoAboutComputer();
        }
        else{
            QMainWindow *w = PropertiesWindowFactoryPluginManager::getInstance()->create(QStringList()<<uri);
            //PropertiesWindow *w = new PropertiesWindow(QStringList()<<uri);
            if(this->parentWidget() && this->parentWidget()->isModal()){
                w->setParent(this->parentWidget());
            }
            w->show();
        }
    });
    if ((0 != QString::compare(m_uri, "computer:///")) &&
        (0 != QString::compare(m_uri, "filesafe:///"))
            &&(m_item->isVolume())) {
        l.last()->setEnabled(m_item->isMounted());
        if(m_item->getDevice().contains("/dev/sr")){/* 光盘在刻录数据、镜像等操作时,即若处于busy状态时，该菜单置灰不可用。 */
            l.last()->setDisabled(FileUtils::isBusyDevice(m_item->getDevice()));
        }
    }

    return l;
}

void SideBarMenu::gotoAboutComputer()
{
    QProcess p;
    p.setProgram("/usr/bin/lingmo-control-center");
    //-m About para to show about computer infos, related to bug#88258
    p.setArguments(QStringList()<<"-m" << "About");
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    p.startDetached();
#else
    p.startDetached("/usr/bin/lingmo-control-center", QStringList()<<"-m" << "About");
#endif
    p.waitForFinished(-1);
}

const QList<QAction *> SideBarMenu::constructNetWorkItemActions()
{
    QList<QAction *> l;
    static const QString netWorkUri="network:///";

    /* 共享文件夹无右键菜单'卸载' */
    if (!m_uri.startsWith("file://")) {
        l<<addAction(QIcon::fromTheme("media-eject-symbolic"), tr("Unmount"), this, [=]() {
            m_item->unmount();
        });
        l.last()->setEnabled(m_item->isMounted());
    }
    if(netWorkUri != m_uri){
        l<<addAction(QIcon::fromTheme("preview-file"), tr("Properties"), this, [=]() {
            if((m_item->isVolume())){
                /* 远程服务器 */
                FileEnumerator e;
                e.setEnumerateDirectory("computer:///");
                e.enumerateSync();
                for (auto fileInfo : e.getChildren()) {
                    FileInfoJob infoJob(fileInfo);
                    infoJob.querySync();

                    /* 由远程服务器的targeturi获取uri来调用属性窗口, */
                    QUrl targetUrl(fileInfo.get()->targetUri());
                    QUrl sourceUrl(m_uri);

                    if(sourceUrl.scheme()==targetUrl.scheme() && sourceUrl.host()==targetUrl.host()){/* 相同scheme和host，但port不同时怎么处理呢？ */
                        QString uri = fileInfo.get()->uri();
                        QMainWindow *w = PropertiesWindowFactoryPluginManager::getInstance()->create(QStringList()<<uri);
                        //PropertiesWindow *w = new PropertiesWindow(QStringList()<<uri);
                        if(this->parentWidget() && this->parentWidget()->isModal()){
                            w->setParent(this->parentWidget());
                        }
                        w->show();
                        break;
                    }
                }
            }else{
                /* 共享文件夹 */
                QMainWindow *w = PropertiesWindowFactoryPluginManager::getInstance()->create(QStringList()<<m_uri);
                //PropertiesWindow *w = new PropertiesWindow(QStringList()<<m_uri);
                if(this->parentWidget() && this->parentWidget()->isModal()){
                    w->setParent(this->parentWidget());
                }
                w->show();
            }
        });
        if(m_item->isVolume())
            l.last()->setEnabled(m_item->isMounted());
    }

    return l;

}

QString SideBarMenu::getComputerUriFromUnixDevice(const QString &unixDevice){
    /* volume item,遍历方式获取uri */
    FileEnumerator e;
    e.setEnumerateDirectory("computer:///");
    e.enumerateSync();
    QString uri;
    for (auto fileInfo : e.getChildren()) {
        FileInfoJob infoJob(fileInfo);
        infoJob.querySync();
        /* 由volume的unixDevice获取computer uir */
        auto info = infoJob.getInfo();
        QString device = fileInfo.get()->unixDeviceFile();
        if(device==unixDevice){
            uri = fileInfo.get()->uri();
            break;
        }
    }
    return uri;
}
