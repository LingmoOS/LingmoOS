// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gomokumainwindow.h"
#include "exitdialog/exitdialog.h"

#include <QGraphicsView>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDBusInterface>
#include <QDBusArgument>

#include <DWidgetUtil>
#include <DWidget>
#include <DTitlebar>
#include <DFrame>
#include <QCloseEvent>
#include <DDialog>
#include <QToolTip>

GomokuMainWindow::GomokuMainWindow(QWidget *parent)
    : DMainWindow(parent)
{
    //禁用最大化窗口
    setWindowFlags(windowFlags() & ~ Qt::WindowMaximizeButtonHint);
    setFixedSize(QSize(widgetWidth, widgetHeight));
    setContentsMargins(QMargins(0, 0, 0, 0));

    initCompositingStatus();
    initUI();

    Dtk::Widget::moveToCenter(this);
}

GomokuMainWindow::~GomokuMainWindow()
{
    if (mTitleBar != nullptr) {
        delete  mTitleBar;
        mTitleBar = nullptr;
    }
}

/**
 * @brief GomokuMainWindow::handleQuit 退出
 */
void GomokuMainWindow::handleQuit()
{
    close();
}

//初始化界面
void GomokuMainWindow::initUI()
{
    mTitleBar = titlebar();
    mTitleBar->setFrameShape(DFrame::NoFrame);
    mTitleBar->installEventFilter(this);
//    //目前系统中没有游戏的图标，先这样设置
    mTitleBar->setIcon(QIcon::fromTheme("com.deepin.gomoku"));
    mTitleBar->setAutoFillBackground(true);
    mTitleBar->setAttribute(Qt::WA_TranslucentBackground);
    mTitleBar->setBackgroundTransparent(true);

    QGraphicsView *wcheckerBoard = new QGraphicsView(this);
    wcheckerBoard->setStyleSheet("background: transparent;border:0px");
    wcheckerBoard->setWindowFlags(Qt::FramelessWindowHint);
    wcheckerBoard->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    wcheckerBoard->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    wcheckerBoard->setFixedSize(this->width(), this->height() - titlebar()->height());
    wcheckerBoard->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    checkerboardScene = new CheckerboardScene(0, 0, this->width(), this->height() - titlebar()->height(), wcheckerBoard);
    wcheckerBoard->setScene(checkerboardScene);
    connect(checkerboardScene, &CheckerboardScene::signalSelectChessPopup, this, &GomokuMainWindow::slotSelectChessPopup);
    connect(checkerboardScene, &CheckerboardScene::signalPopupResult, this, &GomokuMainWindow::slotPopupResult);
    connect(checkerboardScene, &CheckerboardScene::signalReplayFunction, this, &GomokuMainWindow::slotReplayPopup);

    //视图阴影
    m_transparentFrame = new DFrame(this);
    m_transparentFrame->setAutoFillBackground(true);
    m_transparentFrame->hide();
    setCentralWidget(wcheckerBoard);

    //tooltip字体大小
    QFont font;
    font.setFamily(Globaltool::instacne()->loadFontFamilyFromFiles(":/resources/font/ResourceHanRoundedCN-Bold.ttf"));
    font.setWeight(QFont::Black);
    font.setPixelSize(Globaltool::instacne()->getFontSize().dialogButton);

    //tooltip背景颜色
    QPalette palette;
    palette.setColor(QPalette::Background, QColor(Qt::black));

    //tooltip设置字体大小和颜色
    QToolTip::setFont(font);
    QToolTip::setPalette(palette);
}

/**
 * @brief GomokuMainWindow::initCompositingStatus 初始化特效窗口状态
 */
void GomokuMainWindow::initCompositingStatus()
{
    QDBusInterface Interface(COMPOSITINGSERVICE, COMPOSITINGPATH, COMPOSITINGSERVICE, QDBusConnection::sessionBus());
    QDBusConnection::sessionBus().connect(COMPOSITINGSERVICE,
                                          COMPOSITINGPATH,
                                          COMPOSITINGSERVICE,
                                          QLatin1String("compositingEnabledChanged"),
                                          this,
                                          SLOT(slotCompositingChanged(bool)));

    compositingStatus = Interface.property("compositingEnabled").toBool();
    qInfo() << __FUNCTION__ << " compositionStatus: " << compositingStatus;
}

//绘制titlebar背景
void GomokuMainWindow::paintTitleBar(QWidget *titlebar)
{
    DGuiApplicationHelper::ColorType themtype = DGuiApplicationHelper::instance()->themeType();
    QColor broundColor;
    if (themtype == DGuiApplicationHelper::ColorType::DarkType) {
        //深色模式标题栏颜色
        broundColor = QColor("#005A5B");
    } else if (themtype == DGuiApplicationHelper::ColorType::LightType) {
        //浅色模式标题栏颜色
        broundColor = QColor("#88C7C9");
    }
    QPainter painter(titlebar);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.save();
    painter.setPen(Qt::NoPen);
    painter.setBrush(broundColor);
    painter.drawRect(titlebar->rect());
    painter.restore();
}

/**
 * @brief playFailMusic 播放失败音乐
 */
void GomokuMainWindow::playWinMusic()
{
    if (checkerboardScene->getMusicPlay())
        QSound::play(":/resources/music/win.wav");
}

/**
 * @brief playWinMusic 播放胜利音乐
 */
void GomokuMainWindow::playFailMusic()
{
    if (checkerboardScene->getMusicPlay())
        QSound::play(":/resources/music/fail.wav");
}

/**
 * @brief GomokuMainWindow::viewtransparentFrame 视图阴影
 */
void GomokuMainWindow::viewtransparentFrame()
{
    DPalette tframepa = m_transparentFrame->palette();
    QColor tColor = "#000000";
    tColor.setAlphaF(0.3);
    tframepa.setColor(DPalette::Background, tColor);
    m_transparentFrame->setPalette(tframepa);
    m_transparentFrame->setBackgroundRole(DPalette::Background);
    m_transparentFrame->resize(this->width(), this->height());
    m_transparentFrame->show();
}

//过滤titlebar绘制事件
bool GomokuMainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == mTitleBar) {
        if (event->type() == QEvent::Paint) {
            paintTitleBar(mTitleBar);
            return true;
        }
    }
    return DMainWindow::eventFilter(watched, event);
}

/**
 * @brief GomokuMainWindow::slotSelectChessPopup 选择棋子弹窗
 */
void GomokuMainWindow::slotSelectChessPopup()
{
    m_selectChess = new Selectchess(compositingStatus, this);
    m_selectChess->setSelectChess(userChessColor);
    connect(this, &GomokuMainWindow::signalCompositingChanged, m_selectChess, &Selectchess::slotCompositingChanged);
    connect(m_selectChess, &Selectchess::signalButtonOKClicked, this, [ = ] {
        userChessColor = m_selectChess->getSelsectChess();
        if (userChessColor == chess_white)
        {
            checkerboardScene->setSelectChess(chess_white, chess_black);
        } else if (userChessColor == chess_black)
        {
            checkerboardScene->setSelectChess(chess_black, chess_white);
        }
        checkerboardScene->setGameState(GameState::gamePlaying); //点击OK游戏状态为游戏中
        checkerboardScene->selsectChessOK();
        checkerboardScene->startGame();
    });
    connect(checkerboardScene, &CheckerboardScene::signalCloSelectPopup, m_selectChess, &Selectchess::slotCloseSelectPopup);
    viewtransparentFrame();
    if (qApp->platformName() == "dwayland" || qApp->property("_d_isDwayland").toBool()) {
        m_selectChess->exec();
    } else {
        m_selectChess->show();

        setEnabled(false);
        m_selectChess->setEnabled(true);
        //事件循环进入阻塞状态
        QEventLoop loop;
        connect(m_selectChess, &Selectchess::signalButtonOKClicked, &loop, &QEventLoop::quit);
        connect(m_selectChess, &Selectchess::signalDialogClose, &loop, &QEventLoop::quit);
        connect(m_selectChess, &Selectchess::finished, &loop, &QEventLoop::quit);
        loop.exec();
        setEnabled(true);
    }

    m_transparentFrame->hide();
    if (m_selectChess != nullptr) {
        delete m_selectChess;
        m_selectChess = nullptr;
    }
}

/**
 * @brief GomokuMainWindow::slotReplayPopup 重玩弹窗
 */
void GomokuMainWindow::slotReplayPopup()
{
    ExitDialog exitDialog(compositingStatus, this);
    connect(this, &GomokuMainWindow::signalCompositingChanged, &exitDialog, &ExitDialog::slotCompositingChanged);
    viewtransparentFrame();
    if (qApp->platformName() == "dwayland" || qApp->property("_d_isDwayland").toBool()) {
        exitDialog.exec();
    } else {
        exitDialog.show();
        setEnabled(false);
        exitDialog.setEnabled(true);
        //事件循环阻塞
        QEventLoop loop;
        connect(&exitDialog, &ExitDialog::signalClicked, &loop, &QEventLoop::quit);
        connect(&exitDialog, &ExitDialog::finished, &loop, &QEventLoop::quit);
        ifDialog = true;
        loop.exec();
    }

    m_transparentFrame->hide();

    if (exitDialog.getResult() == BTType::BTExit) { //按钮状态是退出状态
        slotReplayFunction();
    } else {
        exitDialog.close();
    }

    setEnabled(true);
    ifDialog = false;
}

/**
 * @brief GomokuMainWindow::slotReplayFunction 重玩功能
 */
void GomokuMainWindow::slotReplayFunction()
{
    checkerboardScene->setGameState(GameState::gameStart); //点击再来一次游戏状态为新游戏
    checkerboardScene->replayFunction();
    slotSelectChessPopup();
}

/**
 * @brief CheckerboardScene::slotPopupResult 游戏结束弹窗
 * @param result 结果
 */
void GomokuMainWindow::slotPopupResult(ChessResult result)
{
    m_resultPopUp = new Resultpopup(compositingStatus, this);
    connect(this, &GomokuMainWindow::signalCompositingChanged, m_resultPopUp, &Resultpopup::slotCompositingChanged);
    connect(m_resultPopUp, &Resultpopup::signalGameAgain, this, &GomokuMainWindow::slotReplayFunction);

    connect(m_resultPopUp, &Resultpopup::signalHaveRest, this, [ = ] {
        checkerboardScene->setStartPauseStatus();
    });
    int userChessColor = checkerboardScene->getUserChessColor();
    //判断用户棋子颜色
    if (userChessColor == chess_black) {
        //判断是哪个颜色的棋子赢了
        if (result == black_win) {
            //胜利弹窗
            m_resultPopUp->setHasWin(true);
            //播放胜利音效
            playWinMusic();
        } else if (result == white_win || result == tie) {
            //失败或平局均为失败弹窗
            //失败弹窗
            m_resultPopUp->setHasWin(false);
            //播放失败音效
            playFailMusic();
        }
    } else if (userChessColor == chess_white) {
        if (result == black_win || result == tie) {
            m_resultPopUp->setHasWin(false);
            playFailMusic();
        } else if (result == white_win) {
            m_resultPopUp->setHasWin(true);
            playWinMusic();
        }
    }
    m_resultPopUp->popupShow();
}

/**
 * @brief GomokuMainWindow::slotCompositingChanged 特效窗口状态改变
 * @param status 是否开启特效窗口
 */
void GomokuMainWindow::slotCompositingChanged(bool status)
{
    compositingStatus = status;
    emit signalCompositingChanged(compositingStatus);
    qInfo() << __FUNCTION__ << " compositionStatusChanged now is: " << compositingStatus;;
}

/**
 * @brief GomokuMainWindow::changeEvent
 */
void GomokuMainWindow::changeEvent(QEvent *event)
{
    if (event->type() != QEvent::WindowStateChange) return;
    if (this->windowState().testFlag(Qt::WindowMinimized) && checkerboardScene) {
        checkerboardScene->stopGAme();
    }
    DMainWindow::changeEvent(event);
}

/**
 * @brief GomokuMainWindow::closeEvent 关闭事件
 * @param event
 */
void GomokuMainWindow::closeEvent(QCloseEvent *event)
{
    //判断是游戏中还是新游戏状态下的关闭
    if (checkerboardScene->getGameState() == GameState::gameStart ||
            checkerboardScene->getGameState() == GameState::gameOver) {
        event->accept();
        return;
    }


    ExitDialog exitDialog(compositingStatus, this);
    connect(this, &GomokuMainWindow::signalCompositingChanged, &exitDialog, &ExitDialog::slotCompositingChanged);
    if (!ifDialog) {
        viewtransparentFrame();

        //判断Wayland环境下直接设置弹窗为模态.
        if (qApp->platformName() == "dwayland" || qApp->property("_d_isDwayland").toBool()) {
            exitDialog.exec();
        } else {
            exitDialog.show();
            //事件循环进入阻塞状态
            QEventLoop loop;
            connect(&exitDialog, &ExitDialog::signalClicked, &loop, &QEventLoop::quit);
            connect(&exitDialog, &ExitDialog::finished, &loop, &QEventLoop::quit);
            loop.exec();
        }
        m_transparentFrame->hide();
    }
    if (exitDialog.getResult() == BTType::BTExit) { //按钮状态是退出状态
        event->accept(); //事件接受
    } else {
        event->ignore(); //事件忽略
    }
}
