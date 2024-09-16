// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SINGLEITEM_H
#define SINGLEITEM_H

#include <QWidget>
#include <QLabel>
#include <QIcon>
#include <dtkwidget_global.h>

DWIDGET_BEGIN_NAMESPACE
class DCommandLinkButton;
DWIDGET_END_NAMESPACE

class SingleItem:public QWidget
{
    Q_OBJECT
public:
    explicit SingleItem(QWidget *parent = nullptr);

    void SetIcon(const QString &iconName);

    void SetText(const QString &text);

    void SetLinkText(const QString &btntext);
Q_SIGNALS:
    void clicked();
private:
    void InitUI();

    void InitConnection();
private:
    QLabel *m_labelIcon;
    QLabel *m_labelText;
    Dtk::Widget::DCommandLinkButton *m_btnLink;
};

#endif // SINGLEITEM_H
