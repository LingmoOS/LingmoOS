// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GOMOKUMAINWINDOW_H
#define GOMOKUMAINWINDOW_H

#include "checkerboard/checkerboardscene.h"
#include "resultpopup/resultpopup.h"
#include "selectchess/selectchess.h"

#include <DMainWindow>
#include <DFrame>

DWIDGET_USE_NAMESPACE

class GomokuMainWindow : public DMainWindow
{
    Q_OBJECT
public:
    explicit GomokuMainWindow(QWidget *parent = nullptr);
    ~GomokuMainWindow() override;

    void handleQuit();

public:
    bool eventFilter(QObject *watched, QEvent *event) override;

protected:
    void changeEvent(QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private:
    void initUI();
    void initCompositingStatus();
    void paintTitleBar(QWidget *titlebar);
    void playWinMusic();
    void playFailMusic();
    void viewtransparentFrame();
signals:
    void signalCompositingChanged(bool status);

private slots:
    void slotSelectChessPopup();
    void slotReplayPopup();
    void slotReplayFunction();
    void slotPopupResult(ChessResult result);
    void slotCompositingChanged(bool status);

private:
    int userChessColor = 1; //用户选择棋子颜色, 默认为黑色
    bool compositingStatus = false; //是否开启特效窗口
    DFrame *m_transparentFrame = nullptr; //视图阴影
    DTitlebar *mTitleBar = nullptr;
    CheckerboardScene *checkerboardScene = nullptr;
    Resultpopup *m_resultPopUp = nullptr;
    Selectchess *m_selectChess = nullptr;
    bool ifDialog = false;
};

#endif // GOMOKUMAINWINDOW_H
