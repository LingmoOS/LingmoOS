// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VNOTE2SICONBUTTON_H
#define VNOTE2SICONBUTTON_H

#include <DApplicationHelper>
#include <DFloatingButton>

#include <QMouseEvent>

DWIDGET_USE_NAMESPACE

class VNote2SIconButton : public DFloatingButton
{
    Q_OBJECT
public:
    explicit VNote2SIconButton(const QString &normal, const QString &press, QWidget *parent = nullptr);

    enum {
        Invalid = -1,
        Normal,
        Press,
        //        Disabled, //Not support this state now
        MaxState
    };
    bool isPressed() const;
    //设置按钮状态
    void setState(int state);
signals:

public slots:
protected:
    //按键处理
    void keyPressEvent(QKeyEvent *e) override;
    //鼠标释放
    void mouseReleaseEvent(QMouseEvent *event) override;
    //更新图标
    void updateIcon();
    //设置图标是否区分主题
    void setCommonIcon(bool isCommon);

    QString m_icons[MaxState];
    int m_state {Normal};

    bool m_useCommonIcons {true};
};

#endif // VNOTE2SICONBUTTON_H
