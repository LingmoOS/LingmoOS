// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef USERLOGIN_H
#define USERLOGIN_H

#include "dsettingsdialog.h"
#include "doanetworkdbus.h"
#include <DLabel>
#include <DIconButton>
#include <DPushButton>
#include <QNetworkAccessManager>
DWIDGET_USE_NAMESPACE

//配置界面账户登录部件
class UserloginWidget: public QWidget
{
    Q_OBJECT
public:
    explicit UserloginWidget(QWidget *parent = nullptr);
    virtual ~UserloginWidget();

    static QPair<QWidget *, QWidget *> createloginButton(QObject *obj);

signals:

public slots:
    void slotLoginBtnClicked();
    void slotLogoutBtnClicked();
    void slotAccountUpdate();
    //网络图片请求完成事件
    void slotReplyPixmapLoad(QNetworkReply *);
    // 网络状态发生改变
    void slotNetworkStateChange(DOANetWorkDBus::NetWorkState state);


private:
    void initView();
    void initConnect();

    QPixmap pixmapToRound(const QPixmap &src, int radius);
private:
    bool m_loginStatus;
    bool m_isManualQuit = false;
    DLabel *m_userNameLabel = nullptr;
    DIconButton *m_buttonImg = nullptr;
    QPushButton *m_buttonLogin = nullptr;
    QPushButton *m_buttonLoginOut = nullptr;
    QNetworkAccessManager *m_networkManager;
    DOANetWorkDBus   * m_ptrDoaNetwork;
};

#endif // USERLOGIN_H
