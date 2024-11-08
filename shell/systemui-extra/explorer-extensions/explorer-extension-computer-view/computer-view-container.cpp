/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, Tianjin LINGMO Information Technology Co., Ltd.
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

#include "computer-view-container.h"
#include "computer-view.h"
#include "computer-proxy-model.h"
#include "abstract-computer-item.h"
#include "login-remote-filesystem.h"
#include "explorer-drive-rename/drive-rename.h"
#include "FMWindowIface.h"
#include "mount-operation.h"

#include <explorer-qt/file-utils.h>
#include <explorer-qt/format_dialog.h>
#include <explorer-qt/file-info-job.h>
#include <explorer-qt/file-item-model.h>
#include <explorer-qt/file-item-proxy-filter-sort-model.h>
#include <explorer-qt/format-dlg-create-delegate.h>

#ifndef LINGMO_UDF_BURN
#include <explorer-qt/disccontrol.h>
#include <explorer-qt/udfFormatDialog.h>
#else
#include <libkyudfburn/disccontrol.h>
#include "volumeManager.h"
#include "udfAppendBurnDataDialog.h"
using namespace UdfBurn;
#endif

#include <QMenu>
#include <QProcess>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QStyleOption>
#include <QInputDialog>
#include <QStylePainter>
#include <file-info.h>
#include <QApplication>

static void ask_question_cb(GMountOperation *op, char *message, char **choices, Peony::ComputerViewContainer *p_this);
static void ask_password_cb(GMountOperation *op, const char *message, const char *default_user, const char *default_domain, GAskPasswordFlags flags, Peony::ComputerViewContainer *p_this);

static GAsyncReadyCallback mount_enclosing_volume_callback(GFile *volume, GAsyncResult *res, Peony::ComputerViewContainer *p_this)
{
    GError *err = nullptr;
    g_file_mount_enclosing_volume_finish (volume, res, &err);
    if ((nullptr == err) || (g_error_matches (err, G_IO_ERROR, G_IO_ERROR_ALREADY_MOUNTED))) {
        qDebug() << "login successful!";
        Q_EMIT p_this->updateWindowLocationRequest(p_this->m_remote_uri);
    } else if (err->message) {
        qDebug() << "login remote error: " <<err->code<<err->message<<err->domain;
        QString showMessage(err->message);
        if (showMessage.startsWith("Message recipient")) {
            showMessage = QObject::tr("Message recipient disconnected from message bus without replying!");
        }
        QMessageBox::warning(nullptr, QObject::tr("log remote error"), showMessage, QMessageBox::Ok);
    }

    if (nullptr != err) {
        g_error_free(err);
    }

    return nullptr;
}

void aborted_cb(GMountOperation *op, Peony::ComputerViewContainer *p_this)
{
    g_mount_operation_reply(op, G_MOUNT_OPERATION_ABORTED);
    p_this->disconnect();
}

#include "explorer-qt/file-enumerator.h"
static QString getComputerUriFromUri(const QString &uri){
    /* volume item,遍历方式获取uri */
    FileEnumerator e;
    e.setEnumerateDirectory("computer:///");
    e.enumerateSync();
    QString computerUri;
    for (auto fileInfo : e.getChildren()) {
        FileInfoJob infoJob(fileInfo);
        infoJob.querySync();
        /* 由item的uri获取computer uir */
        auto info = infoJob.getInfo();
        if(fileInfo.get()->targetUri() == uri && !uri.isEmpty()){
            computerUri = fileInfo.get()->uri();
            break;
        }
    }
    return computerUri;
}

Peony::ComputerViewContainer::ComputerViewContainer(QWidget *parent) : DirectoryViewWidget(parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    m_op = g_mount_operation_new();
    g_signal_connect (m_op, "aborted", G_CALLBACK (aborted_cb), this);
    g_signal_connect (m_op, "ask-question", G_CALLBACK(ask_question_cb), this);
    g_signal_connect (m_op, "ask-password", G_CALLBACK (ask_password_cb), this);

    connect(this, &QWidget::customContextMenuRequested, this, [=](const QPoint &pos){
        auto selectedIndexes = m_view->selectionModel()->selectedIndexes();
        auto index = m_view->indexAt(pos);
        if (!selectedIndexes.contains(index))
            m_view->clearSelection();

        if (index.isValid()) {
            m_view->selectionModel()->select(index, QItemSelectionModel::SelectCurrent);
        }

        QMenu menu(this);
        auto model = static_cast<ComputerProxyModel *>(m_view->model());
        QStringList uris;
        QList<AbstractComputerItem *> items;
        for (auto index : m_view->selectionModel()->selectedIndexes()) {
            auto item = model->itemFromIndex(index);
            uris<<item->uri();
            if (item->uri() == "network:///")
                return;
            items<<item;
        }

        if (items.count() == 0) {

            menu.addAction(tr("Connect a server"), [=](){
                if (nullptr !=m_dlg) {
                    delete m_dlg;
                    m_dlg = nullptr;
                }
                m_dlg = new ConnectServerDialog();

                auto code = m_dlg->exec();
                if (code == QDialog::Rejected) {
                    return;
                }
                GFile* m_volume = g_file_new_for_uri(m_dlg->uri().toUtf8().constData());
                m_remote_uri = m_dlg->uri();

                if (nullptr != m_serverLoginDlg) {
                    delete m_serverLoginDlg;
                    m_serverLoginDlg = nullptr;
                }
                m_serverLoginDlg = new ConnectServerLogin(m_remote_uri);
                g_file_mount_enclosing_volume(m_volume, G_MOUNT_MOUNT_NONE, m_op, nullptr, GAsyncReadyCallback(mount_enclosing_volume_callback), this);
            });
        } else if (items.count() == 1
                   &&  (items.first()->uri() != "" ||items.first()->m_unixDeviceName.contains("/dev/sr"))
                   && items.first()->uri() != "network:///") {
            //bool isWayland = qApp->property("isWayland").toBool(); // related to #105070
            auto item = items.first();
            bool unmountable = item->canUnmount() && !item->property("isKydrive").toBool();
            if(!item->canEject()){
                menu.addAction(tr("Unmount"), [=](){
                    item->unmount(G_MOUNT_UNMOUNT_NONE);
                });
                menu.actions().first()->setEnabled(unmountable);
            }
            //fix bug#175330, wayland should be the same with mainline version
            /* else if (isWayland) {
                menu.addAction(tr("Unmount"), [=](){
                    item->unmount(G_MOUNT_UNMOUNT_NONE);
                });
                menu.actions().first()->setEnabled(unmountable);
            }*/
            /*eject function for volume. fix #18216*/
            if(item->canEject()){
                auto ejectAction = menu.addAction(tr("Eject"), [=](){
                    item->eject(G_MOUNT_UNMOUNT_NONE);
                });
                QString unixDevice = item->m_unixDeviceName;
                if(unixDevice.contains("/dev/sr")){/* 光盘在刻录数据、镜像等操作时,即若处于busy状态时，将弹出菜单置灰不可用。link to bug#143293. */
                    menu.actions().first()->setDisabled(FileUtils::isBusyDevice(unixDevice));
                }
            }

            auto uri = item->uri();
            QString unixDevice = item->m_unixDeviceName;

            auto realUri = m_view->tryGetVolumeRealUriFromUri(uri);
            if (!realUri.isEmpty()) {
                uri = realUri;
            }

            if(uri.startsWith("file://") || !unixDevice.isEmpty()) {
                uri = getComputerUriFromUri(uri);
            }
            auto info = FileInfo::fromUri(uri);
            if (info->displayName().isEmpty() || info->targetUri().isEmpty()) {
                FileInfoJob j(info);
                j.querySync();
            }

            auto mount = VolumeManager::getMountFromUri(info->targetUri());
            //fix bug#52491, CDROM and DVD can format issue

            //fix bug#140543, remote redirection volume can be format
            if (nullptr != mount) {
                auto volume = VolumeManager::getVolumeFromMount(mount);
                if (mount && volume) {
                    QString unixDevice = FileUtils::getUnixDevice(info->uri());
                    if (!unixDevice.isNull() && unixDevice.contains("/dev/sr")
                            &&!unixDevice.startsWith("/dev/bus/usb")
                            && info->isVolume()) {
                        auto fdMenu = menu.addAction(tr("format"));
                        fdMenu->setEnabled(false);
#ifndef LINGMO_UDF_BURN
                        DiscControl *discControl = new DiscControl(unixDevice);
                        if(discControl->work()){
                           connect(discControl, &DiscControl::workFinished, [=](DiscControl *discCtrl){
                               connect(fdMenu, &QAction::triggered, [=](){
                                   UdfFormatDialog *udfFormatDlg = FormatDlgCreateDelegate::getInstance()->createUdfDlg(uri, discCtrl);
                                   udfFormatDlg->show();
                               });
                               qDebug() << unixDevice << "supportUdf values:" << discCtrl->supportUdf();
                               fdMenu->setEnabled(discCtrl->supportUdf());
                           });
                        }
#else
                        UdfBurn::DiscControl *discControl = new UdfBurn::DiscControl(unixDevice);
                        if(discControl->work()){
                           connect(discControl, &UdfBurn::DiscControl::workFinished, [=](UdfBurn::DiscControl *discCtrl){
                               connect(fdMenu, &QAction::triggered, [=](){
                                   UdfBurn::UdfFormatDialogWrapper *udfFormatDlg = FormatDlgCreateDelegate::getInstance()->createUdfDlgWrapper(uri, discCtrl);
                                   udfFormatDlg->show();
                               });
                               qDebug() << unixDevice << "LINGMO_UDF_BURN supportUdf values:" << discCtrl->supportUdf();
                               fdMenu->setEnabled(discControl->supportUdf());
                           });
                        }
#endif
                    } else if (! unixDevice.isNull() && ! unixDevice.contains("/dev/sr")
                            &&!unixDevice.startsWith("/dev/bus/usb")
                            && info->isVolume() && info->canUnmount()/* && info->targetUri() != "file:///data"*/) {
                        auto fdMenu = menu.addAction(tr("format"), [=] () {
                            Format_Dialog *fd = FormatDlgCreateDelegate::getInstance()->createUDiskDlg(uri, nullptr);
                            fd->show();
                        });
                        if (! mount) {
                            fdMenu->setEnabled(false);
                        }
                    }
                }
            }

            if (qApp->property("deviceRenamePluginLoaded").toBool()) {
                // add drive rename action, link to: #105070
                auto driveRenamePlugin = new DriveRename(this);
                QStringList fakeUrls;
                fakeUrls<<uri;

                menu.addActions(driveRenamePlugin->menuActions(Peony::MenuPluginInterface::Type::SideBar, "computer:///", fakeUrls));
            }

            if (!item->uri().startsWith("network://")) {
                auto a = menu.addAction(tr("Property"), [=]() {
                    if (uri.isNull()) {
                        QMessageBox::warning(0, 0, tr("You have to mount this volume first"));
                    } else {
                        QProcess p;
                        p.setProgram("/usr/bin/explorer");
                        QStringList args;
                        args << "-p" << uri;
                        p.setArguments(args);
//                    p.startDetached();
                        p.startDetached(p.program(), p.arguments());
                    }
                });
                a->setEnabled(!uri.isNull());
                if(item->m_unixDeviceName.contains("/dev/sr")){/* 光盘在刻录数据、镜像等操作时,即若处于busy状态时，该菜单置灰不可用。link to bug#143293. */
                    a->setDisabled(FileUtils::isBusyDevice(item->m_unixDeviceName));
                }
            }

            FMWindowIface *windowIface = dynamic_cast<FMWindowIface *>(this->topLevelWidget());
            menu.addAction(tr("Open In New Window"), [=](){
                auto targetUri = info->targetUri();
                if(targetUri.isEmpty() && "computer:///lingmo-data-volume" == info->uri()){
                    targetUri = "file:///data";
                }
                auto newWindow = windowIface->create(targetUri);
                dynamic_cast<QWidget *>(newWindow)->show();
            });

            if (!qApp->property("tabletMode").toBool()) {
                menu.addAction(tr("Open In New Tab"), [=](){
                    auto targetUri = info->targetUri();
                    if(targetUri.isEmpty() && "computer:///lingmo-data-volume" == info->uri()){
                        targetUri = "file:///data";
                    }
                    windowIface->addNewTabs(QStringList()<<targetUri);
                });
            }
        } else if(items.first()->uri() != "" && items.first()->uri() != "network:///"){
            qDebug() << "unable Property uri:" <<items.first()->uri();
            menu.addAction(tr("Property"));
            menu.actions().first()->setEnabled(false);
        }

        if (!menu.isEmpty())
            menu.exec(mapToGlobal(pos));
    });
}

static void ask_question_cb(GMountOperation *op, char *message, char **choices, Peony::ComputerViewContainer *p_this)
{
    qDebug()<<"ask question cb:"<<message;
    Q_UNUSED(p_this);
    QMessageBox *msg_box = new QMessageBox;
    msg_box->setText(message);
    char **choice = choices;
    int i = 0;
    while (*choice) {
        qDebug()<<*choice;
        QPushButton *button = msg_box->addButton(QString(*choice), QMessageBox::ActionRole);
        p_this->m_dlg->connect(button, &QPushButton::clicked, [=]() {
            g_mount_operation_set_choice(op, i);
        });
        *choice++;
        i++;
    }
    //block ui
    msg_box->exec();
    msg_box->deleteLater();
    qDebug()<<"msg_box done";
    g_mount_operation_reply (op, G_MOUNT_OPERATION_HANDLED);
}

static void ask_password_cb(GMountOperation *op, const char *message, const char *default_user, const char *default_domain, GAskPasswordFlags flags, Peony::ComputerViewContainer *p_this)
{
    Q_UNUSED(message);
    Q_UNUSED(default_user);
    Q_UNUSED(default_domain);
    Q_UNUSED(flags);
    Q_UNUSED(p_this);

    if(!p_this->m_serverLoginDlg){
        g_mount_operation_reply (op, G_MOUNT_OPERATION_ABORTED);
        return;
    }

    if (p_this->m_serverLoginDlg->property("IsRejected").toBool()) {
        g_mount_operation_reply (op, G_MOUNT_OPERATION_ABORTED);
        return;
    }

    if ((flags & G_ASK_PASSWORD_NEED_PASSWORD)
            && (g_mount_operation_get_username(op) || g_mount_operation_get_password(op))) {
        QString str = "";
        p_this->m_serverLoginDlg->setCurrentPasswd(str);
        p_this->m_serverLoginDlg->setCurrentUserName(str);
    }

    int code = p_this->m_serverLoginDlg->exec();
    g_mount_operation_set_anonymous(op, p_this->m_serverLoginDlg->anonymous());
    if (code == QDialog::Rejected) {
        g_mount_operation_reply (op, G_MOUNT_OPERATION_ABORTED);
        p_this->m_serverLoginDlg->setProperty("IsRejected", true);
        return;
    }

    p_this->m_serverLoginDlg->syncRemoteServer(p_this->m_remote_uri);
    if (!p_this->m_serverLoginDlg->anonymous()) {
        g_mount_operation_set_username(p_this->m_op, p_this->m_serverLoginDlg->user().toUtf8().constData());
        g_mount_operation_set_password(p_this->m_op, p_this->m_serverLoginDlg->password().toUtf8().constData());
        g_mount_operation_set_domain(p_this->m_op, p_this->m_serverLoginDlg->domain().toUtf8().constData());
    }
    g_mount_operation_set_password_save(p_this->m_op,/* dlgLogin.savePassword()? */G_PASSWORD_SAVE_NEVER/*: G_PASSWORD_SAVE_FOR_SESSION*/);
    p_this->m_serverLoginDlg->setProperty("IsRejected", false);

    g_mount_operation_reply (op, G_MOUNT_OPERATION_HANDLED);
}


Peony::ComputerViewContainer::~ComputerViewContainer()
{
    if (nullptr != m_op) {
        g_object_unref(m_op);
    }
    if (nullptr !=m_dlg) {
        delete m_dlg;
        m_dlg = nullptr;
    }
    if (nullptr != m_serverLoginDlg) {
        delete m_serverLoginDlg;
        m_serverLoginDlg = nullptr;
    }
}

const QStringList Peony::ComputerViewContainer::getSelections()
{
    QStringList uris;
    auto model = static_cast<ComputerProxyModel *>(m_view->model());
    for (auto index : m_view->selectionModel()->selectedIndexes()) {
        auto item = model->itemFromIndex(index);
        uris<<item->uri();
    }
    return uris;
}

void Peony::ComputerViewContainer::paintEvent(QPaintEvent *e)
{
    DirectoryViewWidget::paintEvent(e);
}

void Peony::ComputerViewContainer::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
//        if (m_enterAction)
//            m_enterAction->triggered();
        e->accept();
        return;
    }

    QWidget::keyPressEvent(e);
}

void Peony::ComputerViewContainer::bindModel(Peony::FileItemModel *model, Peony::FileItemProxyFilterSortModel *proxyModel)
{
    m_model = model;
    m_proxyModel = proxyModel;
    model->setRootUri("computer:///");
    connect(model, &Peony::FileItemModel::findChildrenFinished, this, &Peony::DirectoryViewWidget::viewDirectoryChanged);

    if (m_view)
        m_view->deleteLater();

    m_view = new ComputerView(this);
    auto layout = new QHBoxLayout;
    layout->addWidget(m_view);
    setLayout(layout);
    Q_EMIT viewDirectoryChanged();

    auto selectionModel = m_view->selectionModel();
    connect(selectionModel, &QItemSelectionModel::selectionChanged, this, &DirectoryViewWidget::viewSelectionChanged);;

    connect(this, &ComputerViewContainer::containerDoubleClicked, this, [=](const QModelIndex &index){
        if (!index.parent().isValid() || m_view->getRightDoubleClickState()){
            //qDebug() << "doubleClicked state" << m_view->getRightDoubleClickState();
            m_view->setRightDoubleClickState(false);
            return;
        }

        auto model = static_cast<ComputerProxyModel *>(m_view->model());
        auto item = model->itemFromIndex(index);
        if (!item->uri().isEmpty()) {
            item->check();
            qDebug() << "doubleClicked updateWindowLocationRequest:"<<item->uri();
            this->updateWindowLocationRequest(item->uri());
        } else {
            item->mount();
        }
    });

    connect(m_view, &QAbstractItemView::activated, this, [=](const QModelIndex &index) {
        //平板模式下，长按打开文件处理
        if (m_view->m_touch_active_timer->isActive()) {
            auto costTime = m_view->m_touch_active_timer->interval() - m_view->m_touch_active_timer->remainingTime();
            if (costTime > qApp->doubleClickInterval()) {
                m_view->m_touch_active_timer->stop();
                return;
            }
        }

        //when selections is more than 1, let mainwindow to process
        if (m_view->selectionModel()->selectedIndexes().count() != 1)
            return;

        Q_EMIT containerDoubleClicked(m_view->selectionModel()->selectedIndexes().first());
        m_view->m_touch_active_timer->stop();
    });
//    m_enterAction = new QAction(this);
//    m_enterAction->setShortcut(Qt::Key_Enter);
//    addAction(m_enterAction);

//    connect(m_enterAction, &QAction::triggered, this, [=](){
//        if (m_view->selectionModel()->selectedIndexes().count() == 1) {
//            Q_EMIT m_view->doubleClicked(m_view->selectionModel()->selectedIndexes().first());
//        }
//    });

    connect(m_view, &ComputerView::updateLocationRequest, this, &ComputerViewContainer::updateWindowLocationRequest);
}

void Peony::ComputerViewContainer::beginLocationChange()
{
    Q_EMIT viewDirectoryChanged();

    m_view->refresh();
}

void Peony::ComputerViewContainer::stopLocationChange()
{
    Q_EMIT viewDirectoryChanged();
}
