// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXITDIALOG_H
#define EXITDIALOG_H

#include "constants.h"

#include <QWidget>
#include <DDialog>
#include <QPixmap>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>


DWIDGET_USE_NAMESPACE
class ExitDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ExitDialog(bool compositing, QWidget *parent = nullptr);
    ~ExitDialog();
    inline BTType getResult() {return result;}
    inline void setResult(BTType r) {result = r;}

public:
    BTType result; //用于区分那个按钮的点击，让主界面退出游戏

private:
    QPixmap backgroundQPixmap;
    bool compositingStatus = false; //是否开启特效窗口
    QVBoxLayout *m_mainLayout = nullptr; //弹窗主布局


private:
    void initUI();
    void initBackgroundPix();
    void paintEvent(QPaintEvent *event);

signals:
    void signalClicked(); //点击信号

public slots:
    void soltDialogClose();
    void soltGameExit();
    void slotCompositingChanged(bool compositing);

};

#endif // EXITDIALOG_H
