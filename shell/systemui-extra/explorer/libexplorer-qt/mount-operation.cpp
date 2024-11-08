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

#include "mount-operation.h"
//#include "connect-server-dialog.h"
#include "connect-to-server-dialog.h"
#include "gerror-wrapper.h"

#include <QMessageBox>
#include <QPushButton>
#include <QUrl>

#include <QLineEdit>

#include <QDebug>

using namespace Peony;

MountOperation::MountOperation(QString uri, QObject *parent) : QObject(parent)
{
    m_volume = g_file_new_for_uri(uri.toUtf8().constData());
    m_op = g_mount_operation_new();
    m_cancellable = g_cancellable_new();
}

MountOperation::~MountOperation()
{
    disconnect();
    //g_object_disconnect (G_OBJECT (m_op), "any_signal::signal_name", nullptr);
    g_signal_handlers_disconnect_by_func(m_op, (void *)G_CALLBACK(ask_password_cb), nullptr);
    g_signal_handlers_disconnect_by_func(m_op, (void *)G_CALLBACK(ask_question_cb), nullptr);
    g_signal_handlers_disconnect_by_func(m_op, (void *)G_CALLBACK(aborted_cb), nullptr);
    g_object_unref(m_volume);
    g_object_unref(m_op);
    g_object_unref(m_cancellable);

    //g_list_free_full(m_errs, GDestroyNotify(g_error_free));
    m_dlg->deleteLater();
}

void MountOperation::cancel()
{
    g_cancellable_cancel(m_cancellable);
    g_object_unref(m_cancellable);
    m_cancellable = g_cancellable_new();
    Q_EMIT cancelled();
    if (m_auto_delete)
        deleteLater();
}

void MountOperation::start()
{
    gchar* urit = g_file_get_uri(m_volume);
    QUrl uri = QUrl(urit);
    qDebug() << "start uri path:"<<uri.toString();
    //fix bug#214724, 214924， access smb-root error issue
    if (uri.scheme() != "mtp" && uri.toString() != "smb:///") {
        ConnectServerLogin* dlg = new ConnectServerLogin(urit);
        m_dlg = dlg;
        //block ui
        auto code = dlg->exec();
        if (code == QDialog::Accepted) {
            g_mount_operation_set_username(m_op, dlg->user().toUtf8().constData());
            g_mount_operation_set_password(m_op, dlg->password().toUtf8().constData());
            g_mount_operation_set_domain(m_op, dlg->domain().toUtf8().constData());
            g_mount_operation_set_anonymous(m_op, dlg->anonymous());
            //TODO: when FileEnumerator::prepare(), trying mount volume without password dialog first.
            g_mount_operation_set_password_save(m_op, /*dlg->savePassword()? G_PASSWORD_SAVE_FOR_SESSION: */G_PASSWORD_SAVE_NEVER);
        }
        if (code == QDialog::Rejected) {
            cancel();
            QMessageBox msg;
            msg.setIcon(QMessageBox::Critical);
            msg.setText(tr("Operation Cancelled"));
            msg.exec();
            return;
        }
    }

    g_file_mount_enclosing_volume(m_volume, G_MOUNT_MOUNT_NONE, m_op, m_cancellable, GAsyncReadyCallback(mount_enclosing_volume_callback), this);

    g_signal_connect (m_op, "ask-question", G_CALLBACK(ask_question_cb), this);
    g_signal_connect (m_op, "ask-password", G_CALLBACK (ask_password_cb), this);
    g_signal_connect (m_op, "aborted", G_CALLBACK (aborted_cb), this);

    if (nullptr != urit)  g_free(urit);
}

#include "global-settings.h"
GAsyncReadyCallback MountOperation::mount_enclosing_volume_callback(GFile *volume,
        GAsyncResult *res,
        MountOperation *p_this)
{
    GError *err = nullptr;
    g_file_mount_enclosing_volume_finish (volume, res, &err);

    if (err) {
        qDebug()<<err->code<<err->message<<err->domain;
        auto errWarpper = GErrorWrapper::wrapFrom(err);
        p_this->finished(errWarpper);
    } else{
        QUrl url = QUrl(g_file_get_uri(volume));
        p_this->m_dlg->syncRemoteServer(url);
        p_this->finished(nullptr);
    }

    if (p_this->m_auto_delete) {
        p_this->disconnect();
        p_this->deleteLater();
    }
    return nullptr;
}

void MountOperation::ask_question_cb(GMountOperation *op,
                                     char *message, char **choices,
                                     MountOperation *p_this)
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
        connect(button, &QPushButton::clicked, [=]() {
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

void MountOperation::ask_password_cb(GMountOperation *op,
                                     const char *message,
                                     const char *default_user,
                                     const char *default_domain,
                                     GAskPasswordFlags flags,
                                     MountOperation *p_this)
{
    Q_UNUSED(message);
    Q_UNUSED(default_user);
    Q_UNUSED(default_domain);
    Q_UNUSED(flags);
    Q_UNUSED(p_this);

    //block ui
    if (p_this->m_dlg->anonymous()) {
        // try login anonymous once, if not successed, ask password.
        p_this->m_dlg->m_usr_btn_usr->click();
        g_mount_operation_reply (op, G_MOUNT_OPERATION_HANDLED);
        return;
    } else {
        // try get password and login once, if not successed, show message and
        // require input password again.
        if (!p_this->m_dlg->password().isEmpty() && !p_this->m_dlg->m_reg_usr_passwd_editor->property("password").toString().isEmpty()) {
            p_this->m_dlg->m_reg_usr_passwd_editor->setText(nullptr);/* 去掉，因为记住密码时需要保存用户名密码 */
            g_mount_operation_reply (op, G_MOUNT_OPERATION_HANDLED);
            return;
        } else {
            qDebug() <<__func__ << __LINE__ << message;
            QUrl url(p_this->m_dlg->m_remoteIP);
            QString showMessage(message);
            if (showMessage.contains(url.host())){
                showMessage = QString(tr("Login failed, unknown username or password error, please re-enter!"));
                QString str = "";
                p_this->m_dlg->setCurrentUserName(str);
            }
            QMessageBox::information(0, 0, showMessage);
            auto code = p_this->m_dlg->exec();
            auto dlg = p_this->m_dlg;
            if (code == QDialog::Accepted) {
                g_mount_operation_set_username(op, dlg->user().toUtf8().constData());
                g_mount_operation_set_password(op, dlg->password().toUtf8().constData());
                g_mount_operation_set_domain(op, dlg->domain().toUtf8().constData());
                g_mount_operation_set_anonymous(op, dlg->anonymous());
                g_mount_operation_set_password_save(op, /*dlg->savePassword()? G_PASSWORD_SAVE_FOR_SESSION: */G_PASSWORD_SAVE_NEVER);
                g_mount_operation_reply (op, G_MOUNT_OPERATION_HANDLED);
                return;
            }
            if (code == QDialog::Rejected) {
                g_mount_operation_reply (op, G_MOUNT_OPERATION_ABORTED);
                return;
            }
        }
    }

    g_mount_operation_reply (op, G_MOUNT_OPERATION_HANDLED);
}

void MountOperation::aborted_cb(GMountOperation *op,
                                MountOperation *p_this)
{
    g_mount_operation_reply(op, G_MOUNT_OPERATION_ABORTED);
    p_this->disconnect();
    p_this->deleteLater();
}
