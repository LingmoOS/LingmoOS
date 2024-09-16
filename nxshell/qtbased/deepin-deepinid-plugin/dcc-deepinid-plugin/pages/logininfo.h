// Copyright (C) 2016 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "interface/namespace.h"
#include "utils.h"
#include "operation/syncmodel.h"
#include "avatarwidget.h"
#include "downloadurl.h"
#include <dtkwidget_global.h>

DWIDGET_BEGIN_NAMESPACE
class DListView;
class DLabel;
class DLineEdit;
class DToolButton;
DWIDGET_END_NAMESPACE

QT_BEGIN_NAMESPACE
class QObject;
class QWidget;
class QStandardItemModel;
class QVBoxLayout;
class QPushButton;
QT_END_NAMESPACE

class LoginInfoPage : public QWidget
{
    Q_OBJECT
public:
    explicit LoginInfoPage(QWidget *parent = nullptr);
    ~LoginInfoPage() override;

    void setModel(SyncModel *model);
    void onLogin();
    // 使用浏览器打开web端账户中心
    void openWeb();
Q_SIGNALS:
    void requestLogoutUser() const;
    void requestSetFullname(const QString &fullname);
    void requestPullMessage() const;
    void selectIndex(int index);

protected:
    void initUI();
    void initConnection();
    void onUserInfoChanged(const QVariantMap &infos);
    bool eventFilter(QObject *obj, QEvent *event) override;
    void onEditingFinished(const QString& userFullName);
    void onAvatarChanged(const QString &avaPath);
    void onUserInfoListChanged(const QList<QPair<QString, QString>> &moduleTs);
    void setAvatarPath(const QString &avatarPath);
    void onUserUnbindInfoChanged(const QString& ubid);
    void resizeEvent(QResizeEvent *event) override;
    void initGroupInfo();

private:
    QString handleNameTooLong(const QString &fullName);
    void onResetError(const QString &error);
    QString loadCodeURL();

private:
    QVBoxLayout *m_mainLayout;
    SyncModel *m_model;

    DownloadUrl *m_downloader;
    QString m_avatarPath;
    AvatarWidget *m_avatar;
    bool m_fristLogin; // 首次登录
    QString m_userFullName;

    DTK_WIDGET_NAMESPACE::DLabel *m_username;
    DTK_WIDGET_NAMESPACE::DToolButton *m_editNameBtn;
    DTK_WIDGET_NAMESPACE::DLineEdit *m_inputLineEdit;
    DTK_WIDGET_NAMESPACE::DLabel *m_region;

    // Info ListView
    DTK_WIDGET_NAMESPACE::DListView *m_listView;
    QStandardItemModel *m_listModel;
    QMap<int, QString> m_listIcon;

    QPushButton *m_logoutBtn;
    QPushButton *m_editInfoBtn;
};
