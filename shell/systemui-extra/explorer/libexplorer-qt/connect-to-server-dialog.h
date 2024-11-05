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
 * Authors: Ding Jing <dingjing@kylinos.cn>
 *
 */

#ifndef CONNECTTOSERVERDIALOG_H
#define CONNECTTOSERVERDIALOG_H

#include <QLabel>
#include <QWidget>
#include <QWidget>
#include <QDialog>
#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QRadioButton>
#include <QProxyStyle>
#include "explorer-core_global.h"

namespace Peony {

class PEONYCORESHARED_EXPORT ConnectServerDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ConnectServerDialog(QWidget *parent = nullptr);
    ~ConnectServerDialog();

    QString uri();
    QString getIP();

private:
    void syncUri();
    void setUri(QString uri);
    void addUri (QString uri);
    void removeUri (QString uri);
    void checkConnectIpAndPort(QString uri);

private:
    float           m_widget_margin         = 24;
    QSize           m_widget_size           = QSize (424, 440);
    QVBoxLayout*    m_main_layout           = nullptr;
    QStringList*    m_remote_type;
    QStringList*    m_remote_port;

    QPoint          m_remote_type_pos       = QPoint (24, 70);
    QSize           m_remote_type_size      = QSize (28, 24);
    QLabel*         m_remote_type_label     = nullptr;
    QComboBox*      m_remote_type_edit      = nullptr;
    QLabel*         m_ip_label              = nullptr;
    QLineEdit*      m_ip_edit               = nullptr;
    QLabel*         m_port_label            = nullptr;
    QComboBox*      m_port_editor           = nullptr;
    QGridLayout*    m_remote_layout         = nullptr;

    QSize           m_favorite_list_size    = QSize (376, 144);
    QLabel*         m_favorite_label        = nullptr;
    QListWidget*    m_favorite_list         = nullptr;
    QVBoxLayout*    m_favorite_layout       = nullptr;

    QPushButton*    m_btn_add               = nullptr;
    QPushButton*    m_btn_del               = nullptr;
    QPushButton*    m_btn_conn              = nullptr;
    QHBoxLayout*    m_btn_layout            = nullptr;

    QMap<QString, QVariant>         m_favorite_uri;
    QMap<QString, QListWidgetItem*> m_favorite_widgets;
    bool m_checkIp                           = false;
    bool m_checkPort                         = false;
};


class PEONYCORESHARED_EXPORT ConnectServerLogin : public QDialog
{
    friend class MountOperation;
    Q_OBJECT
public:
    explicit ConnectServerLogin(QString uri, QWidget* parent = nullptr);
    ~ConnectServerLogin();

    QString user();
    QString domain();
    QString password();
    bool anonymous();
    bool savePassword();
    void setCurrentUserName(QString &name);
    void setCurrentPasswd(QString &passwd);

    void syncRemoteServer(const QUrl& url);
    QString getPassWordProperty();

private:
    float           m_widget_margin         = 24;
    QSize           m_widget_size           = QSize (424, 394);
    QSize           m_widget_size_little    = QSize (424, 250);
    QVBoxLayout*    m_main_layout           = nullptr;

    QLabel*         m_tip                   = nullptr;

    QLabel*         m_usr_label             = nullptr;
    QRadioButton*   m_usr_btn_guest         = nullptr;
    QRadioButton*   m_usr_btn_usr           = nullptr;
    QGridLayout*    m_usr_layout            = nullptr;

    QLabel*         m_reg_usr_name_label    = nullptr;
    QLabel*         m_reg_usr_passwd_label  = nullptr;
    QComboBox*      m_reg_usr_name_editor   = nullptr;
    QLineEdit*      m_reg_usr_passwd_editor = nullptr;
    QCheckBox*      m_reg_usr_combox        = nullptr;
    QGridLayout*    m_reg_usr_layout        = nullptr;

    QPushButton*    m_btn_cancel            = nullptr;
    QPushButton*    m_btn_ok                = nullptr;
    QHBoxLayout*    m_btn_layout            = nullptr;

    QString m_remoteIP;

    QMap<QString, QVariant> m_userInfo;

};

class ButtonStyle : public QProxyStyle
{

public:
    static ButtonStyle *getStyle();

    ButtonStyle() : QProxyStyle() {}

    void drawControl(QStyle::ControlElement element,
                     const QStyleOption *option,
                     QPainter *painter,
                     const QWidget *widget = nullptr) const;

    int pixelMetric(PixelMetric metric,
                    const QStyleOption *option = nullptr,
                    const QWidget *widget = nullptr) const override;

    QRect subElementRect(SubElement element,
                         const QStyleOption *option,
                         const QWidget *widget = nullptr) const;
};

};
#endif // CONNECTTOSERVERDIALOG_H
