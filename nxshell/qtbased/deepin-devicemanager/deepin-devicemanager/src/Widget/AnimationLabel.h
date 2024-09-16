// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ANIMATIONLABEL_H
#define ANIMATIONLABEL_H

#include <DLabel>

#include <QWidget>
#include <QPixmap>

DWIDGET_USE_NAMESPACE

/**
 * @brief The AnimationLabel class   用于实现检测界面的动画
 */
class AnimationLabel : public DLabel
{
    Q_OBJECT
    Q_PROPERTY(int icon READ currentIcon WRITE setcurrentIcon)

public:
    explicit AnimationLabel(QWidget * parent = nullptr);

    /**
     * @brief setImageList: 设置图片列表
     * @param iconList:图片列表
     */
    void setImageList(const QStringList &iconList);

    /**
     * @brief setcurrentIcon: 设置当前icon
     * @param iconIndex: 图片Index
     */
    void setcurrentIcon(int iconIndex);

    /**
     * @brief currentIcon: 当前icon
     * @return 图片Index
     */
    int currentIcon();

private:
    QStringList  m_ImageList;       // 图片列表
    int          m_CurrentIndex;    // 当前Index

};

#endif // ANIMATIONLABEL_H
