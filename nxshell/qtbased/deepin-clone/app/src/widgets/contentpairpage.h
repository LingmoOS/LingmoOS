// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef CONTENTPAIRPAGE_H
#define CONTENTPAIRPAGE_H

#include <QWidget>

class ViewBox;
class ContentPairPage : public QWidget
{
    Q_OBJECT

public:
    explicit ContentPairPage(QWidget *parent = 0);

    void setLeftTitle(const QString &title);
    void setRightTitle(const QString &title);

    void setLeftContent(QWidget *widget, bool full = false);
    void setRightContent(QWidget *widget, bool full = false);

    QWidget *leftContent() const;
    QWidget *rightContent() const;

private:
    void init();
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

    ViewBox *m_leftBox;
    ViewBox *m_rightBox;
};

#endif // CONTENTPAIRPAGE_H
