// Copyright (C) 2019 ~ 2021 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QLineEdit>

class QVariantAnimation;

class AnimatedLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit AnimatedLineEdit(QWidget *parent = nullptr);
    virtual ~AnimatedLineEdit() override;

    void startAnimation();

protected:
    virtual void resizeEvent(QResizeEvent *e) override;
    virtual void paintEvent(QPaintEvent *e) override;

Q_SIGNALS:

public Q_SLOTS:
    void updateIcon();

private:
    void initUi();
    void initConnection();

private:
    QVariantAnimation *m_animation;
    QPixmap m_toLeftGlowingImg;
    QPixmap m_toRightGlowingImg;
};
