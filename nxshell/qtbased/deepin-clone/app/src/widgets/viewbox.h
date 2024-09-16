// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef VIEWBOX_H
#define VIEWBOX_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QLabel;
class QFrame;
class QBoxLayout;
QT_END_NAMESPACE

class IconLabel;
class ViewBox : public QWidget
{
    Q_OBJECT

public:
    explicit ViewBox(QWidget *parent = 0);

    void setTitle(const QString &title);
    void setIcon(const QIcon &icon);
    void setContent(QWidget *widget, bool full = false);

    QWidget *content() const;

private:
    void init();
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;

    IconLabel *m_title;
    QFrame *m_borderFrame;
    QBoxLayout *m_borderFrameLayout;
    QWidget *m_contentWidget = NULL;
    bool m_fullContent = false;
};

#endif // VIEWBOX_H
