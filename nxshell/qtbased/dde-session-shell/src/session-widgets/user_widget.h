// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef USERWIDGET_H
#define USERWIDGET_H

#include "userinfo.h"

#include <DBlurEffectWidget>
#include <DLabel>

#include <QWidget>

DWIDGET_USE_NAMESPACE

const int UserFrameWidth = 226;
const int UserFrameHeight = 167;

class UserAvatar;
class QVBoxLayout;
class UserWidget : public QWidget
{
    Q_OBJECT
public:
    explicit UserWidget(QWidget *parent = nullptr);

    void setUser(std::shared_ptr<User> user);

    inline bool isSelected() const { return m_isSelected; }
    void setSelected(bool isSelected);
    void setFastSelected(bool isSelected);

    inline uint uid() const { return m_uid; }
    void setUid(const uint uid);

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void initUI();
    void initConnections();

    void setAvatar(const QString &avatar);
    void updateUserNameLabel();
    void setLoginState(const bool isLogin);

    void updateBlurEffectGeometry();

private:
    bool m_isSelected;
    uint m_uid;

    QVBoxLayout *m_mainLayout; // 登录界面布局

    DBlurEffectWidget *m_blurEffectWidget; // 模糊背景
    UserAvatar *m_avatar;                  // 用户头像

    DLabel *m_loginState;  // 用户登录状态
    DLabel *m_nameLabel;   // 用户名
    QWidget *m_nameWidget; // 用户名控件

    std::shared_ptr<User> m_user;
};

#endif // USERWIDGET_H
