// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef QUICKPANELWIDGET_H
#define QUICKPANELWIDGET_H

#include "commoniconbutton.h"
#include "mediacontroller.h"

#include <QWidget>

#include <DLabel>

DWIDGET_USE_NAMESPACE
class QuickPanelWidget : public QWidget
{
    Q_OBJECT
public:
    QuickPanelWidget(MediaController *controller, QWidget *parent = nullptr);
    ~QuickPanelWidget();

protected:
    void mouseReleaseEvent(QMouseEvent *event);

Q_SIGNALS:
    void clicked();
    void requestHideApplet();

private:
    void init();
    void updateUI();

private:
    MediaController *m_controller;

    DLabel *m_pixmap;
    DLabel *m_titleLab;
    DLabel *m_artistLab;
    CommonIconButton *m_playButton;
    CommonIconButton *m_nextButton;
};

#endif