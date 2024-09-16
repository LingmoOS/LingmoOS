// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GLTESTWINDOW_H
#define GLTESTWINDOW_H

#include "gltestwidget.h"
#include "graphicsdriverproxy.h"

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>


class GLTestWindow : public QWidget
{
    Q_OBJECT

public:
    explicit GLTestWindow(QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *e);

private slots:
    void onAccept();

    void onCancel();

private:
    bool m_exit_gltest;
    GLTestWidget *m_glTestWidget;
    QLabel *m_tipsLabel;
    QPushButton *m_acceptBtn;
    QPushButton *m_cancelBtn;
    ComDeepinDaemonGraphicsDriverInterface *m_graphicsDriver = nullptr;
};

#endif
