// SPDX-FileCopyrightText: 2015 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef USERBUTTON_H
#define USERBUTTON_H

#include <QGraphicsDropShadowEffect>
#include <QGraphicsOpacityEffect>
#include <QPushButton>
#include <QtGui/QPaintEvent>
#include <QVBoxLayout>
#include <QObject>
#include <QLabel>
#include <QPropertyAnimation>
#include <memory>

#include "useravatar.h"

#include "accountsuser_interface.h"

using UserInter = org::deepin::dde::accounts1::User;

static const int USER_ICON_WIDTH = 180;
static const int USER_ICON_HEIGHT = 180;

class User;
class UserButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(double opacity READ opacity WRITE setOpacity NOTIFY opacityChanged)
public:
    enum AvatarSize {
        AvatarSmallSize = 80,
        AvatarNormalSize = 90,
        AvatarLargeSize = 100
    };

    UserButton(std::shared_ptr<User> user, QWidget* parent=0);
    ~UserButton();

    bool selected() const;
    void setSelected(bool selected);

    std::shared_ptr<User> userInfo() const { return m_user; }

signals:
    void opacityChanged();

public slots:
    void setImageSize(const AvatarSize &avatarsize);

    void show();
    void hide();
    void move(const QPoint &position, bool immediately = false);
    void stopAnimation();

    double opacity();
    void setOpacity(double opa);
    void setCustomEffect();
    void addTextShadowAfter();

protected:
    void paintEvent(QPaintEvent* event);

private:
    void initUI();
    void initConnect();
    void addTextShadow(bool isEffective);

private:
    std::shared_ptr<User> m_user;

    bool m_selected;
    UserAvatar* m_userAvatar;
    QLabel* m_userNameLabel;
    QLabel *m_checkedMark;
    QHBoxLayout* m_buttonLayout;
    QHBoxLayout *m_nameLayout;
    QVBoxLayout* m_centralLayout;

    int m_borderWidth = 0;

    double m_opacity;

#ifndef DISABLE_ANIMATIONS
    QPropertyAnimation *m_moveAni;
    QPropertyAnimation* m_showAnimation;
    QPropertyAnimation* m_hideAnimation;
#endif

    QGraphicsOpacityEffect* m_opacityEffect;
};
#endif // UserButton

