// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LOGINVIEW_H
#define LOGINVIEW_H

#include <QWebEngineView>

class LoginView : public QWebEngineView
{
    Q_OBJECT
public:
    explicit LoginView(QWidget *parent = nullptr);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

};

#endif // LOGINVIEW_H
