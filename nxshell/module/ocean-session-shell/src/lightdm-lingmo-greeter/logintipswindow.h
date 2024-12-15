// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOGINTIPSWINDOW_H
#define LOGINTIPSWINDOW_H

#include <QWidget>

class LoginTipsWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LoginTipsWindow(QWidget *parent = 0);
    bool isValid();

Q_SIGNALS:
    void closed();

private:
    void initUI();

private:
    QString m_tips;
    QString m_content;
};

#endif // LOGINTIPSWINDOW_H
