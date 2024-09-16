// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CGROUPBUTTONWGT_H
#define CGROUPBUTTONWGT_H
#include <DToolButton>

#include "attributewidget.h"

DWIDGET_USE_NAMESPACE

class DrawBoard;
class PageScene;
class GroupButtonWidget : public AttributeWgt
{
    Q_OBJECT
public:
    explicit GroupButtonWidget(DrawBoard *drawBoard, QWidget *parent = nullptr);

    void setGroupFlag(bool canGroup, bool canUngroup);
private:
    void updateButtonStatus();

signals:
    void buttonClicked(bool doGroup, bool doUngroup);

private:
    DToolButton *groupButton;
    DToolButton *unGroupButton;
    QLabel *m_titleLabel;
    DrawBoard   *m_drawBoard = nullptr;
    PageScene   *m_currentScene = nullptr;
};

#endif // CGROUPBUTTONWGT_H
