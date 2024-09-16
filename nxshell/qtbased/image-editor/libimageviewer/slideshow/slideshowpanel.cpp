// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "slideshowpanel.h"
#include "service/configsetter.h"
#include "viewpanel/viewpanel.h"
#include "unionimage/baseutils.h"
#include "unionimage/imageutils.h"
#include "unionimage/unionimage.h"
#include "accessibility/ac-desktop-define.h"

#include <QResizeEvent>
#include <QStyleFactory>
#include <QScreen>
#include <QDesktopWidget>
#include <QGuiApplication>
#include <QApplication>

namespace  {
const int DELAY_HIDE_CURSOR_INTERVAL = 3000;
const QColor DARK_BG_COLOR = QColor("#252525");
const QColor LIGHT_BG_COLOR = QColor("#FFFFFF");
const QString SHORTCUTVIEW_GROUP = "SHORTCUTVIEW";
}

DWIDGET_USE_NAMESPACE
namespace  {
const QSize ICON_SIZE = QSize(50, 50);
const int ICON_SPACING = 10;
const int WIDTH = 260;
const int HEIGHT = 81;
}

SlideShowBottomBar::SlideShowBottomBar(QWidget *parent) : DFloatingWidget(parent)
{
    setCursor(Qt::ArrowCursor);
    setFixedSize(WIDTH, HEIGHT);
    QHBoxLayout *hb = new QHBoxLayout();
    hb->setContentsMargins(ICON_SPACING, 0, ICON_SPACING, 0);
    m_preButton = new DIconButton(this);
    AC_SET_OBJECT_NAME(m_preButton, Slider_Pre_Button);
    AC_SET_ACCESSIBLE_NAME(m_preButton, Slider_Pre_Button);
    m_preButton->setFixedSize(ICON_SIZE);
    m_preButton->setIcon(QIcon::fromTheme("dcc_previous_normal"));
    m_preButton->setIconSize(QSize(36, 36));
    m_preButton->setToolTip(tr("Previous"));
    m_preButton->setFocusPolicy(Qt::NoFocus);
    hb->addWidget(m_preButton);
    hb->addSpacing(4);
    connect(m_preButton, &DIconButton::clicked, this, &SlideShowBottomBar::onPreButtonClicked);
    m_playpauseButton = new DIconButton(this);
    m_playpauseButton->setShortcut(Qt::Key_Space);
    AC_SET_OBJECT_NAME(m_playpauseButton, Slider_Play_Pause_Button);
    AC_SET_ACCESSIBLE_NAME(m_playpauseButton, Slider_Play_Pause_Button);
    m_playpauseButton->setFixedSize(ICON_SIZE);
    m_playpauseButton->setIcon(QIcon::fromTheme("dcc_suspend_normal"));
    m_playpauseButton->setIconSize(QSize(36, 36));
    m_playpauseButton->setToolTip(tr("Pause"));
    m_playpauseButton->setFocusPolicy(Qt::NoFocus);
    hb->addWidget(m_playpauseButton);
    hb->addSpacing(4);
    connect(m_playpauseButton, &DIconButton::clicked, this, &SlideShowBottomBar::onPlaypauseButtonClicked);
    //todo屏蔽了全局信号
//    connect(dApp->signalM, &SignalManager::updatePauseButton, this, &SlideShowBottomBar::onUpdatePauseButton);
//    connect(dApp->signalM, &SignalManager::initSlideShowButton, this, &SlideShowBottomBar::onInitSlideShowButton);

    m_nextButton = new DIconButton(this);
    AC_SET_OBJECT_NAME(m_nextButton, Slider_Next_Button);
    AC_SET_ACCESSIBLE_NAME(m_nextButton, Slider_Next_Button);
    m_nextButton->setFixedSize(ICON_SIZE);
    m_nextButton->setIcon(QIcon::fromTheme("dcc_next_normal"));
    m_nextButton->setIconSize(QSize(36, 36));
    m_nextButton->setToolTip(tr("Next"));
    m_nextButton->setFocusPolicy(Qt::NoFocus);
    hb->addWidget(m_nextButton);
    hb->addSpacing(4);
    connect(m_nextButton, &DIconButton::clicked, this, &SlideShowBottomBar::onNextButtonClicked);
    m_cancelButton = new DIconButton(this);
    AC_SET_OBJECT_NAME(m_cancelButton, Slider_Exit_Button);
    AC_SET_ACCESSIBLE_NAME(m_cancelButton, Slider_Exit_Button);
    m_cancelButton->setFixedSize(ICON_SIZE);
    m_cancelButton->setIcon(QIcon::fromTheme("dcc_exit_normal"));
    m_cancelButton->setIconSize(QSize(36, 36));
    m_cancelButton->setToolTip(tr("Exit"));
    m_cancelButton->setFocusPolicy(Qt::NoFocus);
    hb->addWidget(m_cancelButton);
    connect(m_cancelButton, &DIconButton::clicked, this, &SlideShowBottomBar::onCancelButtonClicked);
    setLayout(hb);
}

void SlideShowBottomBar::onPreButtonClicked()
{
    //todo屏蔽了全局信号
//    emit dApp->signalM->updatePauseButton();
//    emit dApp->signalM->updateButton();

    onUpdatePauseButton();
    emit showPrevious();
}

void SlideShowBottomBar::onPlaypauseButtonClicked()
{
    if (!isStop) {
        m_playpauseButton->setIcon(QIcon::fromTheme("dcc_play_normal"));
        m_playpauseButton->setToolTip(tr("Play"));
        isStop = true;
        //todo屏蔽了全局信号
//        emit dApp->signalM->updateButton();
        emit showPause();
    } else {
        m_playpauseButton->setIcon(QIcon::fromTheme("dcc_suspend_normal"));
        m_playpauseButton->setToolTip(tr("Pause"));
        isStop = false;
        //todo屏蔽了全局信号
//        emit dApp->signalM->sigStartTimer();
        emit showContinue();
    }
}

void SlideShowBottomBar::onUpdatePauseButton()
{
    m_playpauseButton->setIcon(QIcon::fromTheme("dcc_play_normal"));
    m_playpauseButton->setToolTip(tr("Play"));
    isStop = true;
}

void SlideShowBottomBar::onInitSlideShowButton()
{
    m_playpauseButton->setIcon(QIcon::fromTheme("dcc_suspend_normal"));
    m_playpauseButton->setToolTip(tr("Pause"));
    isStop = false;
}

void SlideShowBottomBar::onNextButtonClicked()
{
    //todo屏蔽了全局信号
//    emit dApp->signalM->updatePauseButton();
//    emit dApp->signalM->updateButton();

    onUpdatePauseButton();
    emit showNext();
}

void SlideShowBottomBar::onCancelButtonClicked()
{
    emit showCancel();
}

LibSlideShowPanel::LibSlideShowPanel(QWidget *parent) : QWidget(parent)
    , slideshowbottombar(new SlideShowBottomBar(this)), m_animation(new LibImageAnimation(this))
    , m_hideCursorTid(0)
{
    setObjectName(SLIDE_SHOW_WIDGET);
    //setFocusPolicy(Qt::StrongFocus);
    initMenu();
    initConnections();
    setMouseTracking(true);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->addWidget(m_animation);
    this->setLayout(layout);
    qDebug() << QGuiApplication::primaryScreen()->geometry().width();
    //移出屏幕外
    slideshowbottombar->move((QGuiApplication::primaryScreen()->geometry().width() - slideshowbottombar->width()) / 2, QGuiApplication::primaryScreen()->geometry().height());
    //移至顶层
    slideshowbottombar->raise();
}

void LibSlideShowPanel::initConnections()
{
    connect(m_animation, &LibImageAnimation::singleAnimationEnd, this, &LibSlideShowPanel::onSingleAnimationEnd);
//todo屏蔽了全局信号
//    connect(dApp->signalM, &SignalManager::startSlideShow, this, &SlideShowPanel::startSlideShow);
//    connect(dApp->signalM, &SignalManager::sigESCKeyStopSlide, this, &SlideShowPanel::onESCKeyStopSlide);

    connect(slideshowbottombar, &SlideShowBottomBar::showPause, this, &LibSlideShowPanel::onShowPause);
    connect(slideshowbottombar, &SlideShowBottomBar::showContinue, this, &LibSlideShowPanel::onShowContinue);
    connect(slideshowbottombar, &SlideShowBottomBar::showPrevious, this, &LibSlideShowPanel::onShowPrevious);
    connect(slideshowbottombar, &SlideShowBottomBar::showNext, this, &LibSlideShowPanel::onShowNext);
    connect(slideshowbottombar, &SlideShowBottomBar::showCancel, this, &LibSlideShowPanel::backToLastPanel);
}

void LibSlideShowPanel::initMenu()
{
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    m_menu = new DMenu(this);
    m_menu->setStyle(QStyleFactory::create("dlight"));
    appendAction(IdPlayOrPause, tr(slideshowbottombar->m_playpauseButton->toolTip().toStdString().c_str()), "");
    appendAction(IdStopslideshow, tr(slideshowbottombar->m_cancelButton->toolTip().toStdString().c_str()), "");
    connect(m_menu, &QMenu::triggered, this, &LibSlideShowPanel::onMenuItemClicked);
    connect(this, &LibSlideShowPanel::customContextMenuRequested, this, &LibSlideShowPanel::onCustomContextMenuRequested);
}

void LibSlideShowPanel::appendAction(int id, const QString &text, const QString &shortcut)
{
    QAction *ac = new QAction(m_menu);
    ac->setText(text);
    ac->setProperty("MenuID", id);
    ac->setShortcut(QKeySequence(shortcut));
    m_menu->addAction(ac);
    if (id == IdPlayOrPause) {
        connect(slideshowbottombar, &SlideShowBottomBar::showPause, ac, [ = ] {
            ac->setText(tr(slideshowbottombar->m_playpauseButton->toolTip().toStdString().c_str()));
        });
        connect(slideshowbottombar, &SlideShowBottomBar::showContinue, ac, [ = ] {
            ac->setText(tr(slideshowbottombar->m_playpauseButton->toolTip().toStdString().c_str()));
        });
        connect(slideshowbottombar, &SlideShowBottomBar::showNext, ac, [ = ] {
            ac->setText(tr(slideshowbottombar->m_playpauseButton->toolTip().toStdString().c_str()));
        });
        connect(slideshowbottombar, &SlideShowBottomBar::showPrevious, ac, [ = ] {
            ac->setText(tr(slideshowbottombar->m_playpauseButton->toolTip().toStdString().c_str()));
        });
    }
}

void LibSlideShowPanel::backToLastPanel()
{
    m_animation->endSlider();
    showNormal();
    if (0 == m_vinfo.viewMainWindowID) {
        //        m_vinfo.path = m_player->currentImagePath();
        //        m_vinfo.fullScreen = false;
        //        m_vinfo.slideShow = false;
        //        emit dApp->signalM->hideSlidePanel();
        //        emit dApp->signalM->viewImage(m_vinfo);


//        emit dApp->signalM->hideSlidePanel();
//        emit dApp->signalM->showBottomToolbar();
//        emit dApp->signalM->showTopToolbar();
//        QEventLoop loop;
//        QTimer::singleShot(100, &loop, SLOT(quit()));
//        loop.exec();
        //        QString path = m_player->currentImagePath();
        //        emit dApp->signalM->viewImageNoNeedReload(path);
        QString currentpath = m_animation->currentPath();

        //todo屏蔽了全局信号
        emit hideSlidePanel(m_animation->currentPath());
//todo屏蔽了全局信号
//        emit dApp->signalM->viewImageNoNeedReload(currentpath);
    } else {
        //todo屏蔽了全局信号
//        emit dApp->signalM->hideSlidePanel();
    }
    killTimer(m_hideCursorTid);
    m_hideCursorTid = 0;
    this->setCursor(Qt::ArrowCursor);
}

void LibSlideShowPanel::showNormal()
{
    if (m_isMaximized) {
        window()->showNormal();
        window()->showMaximized();
    } else {
        window()->showNormal();
    }
}

void LibSlideShowPanel::showFullScreen()
{
    m_isMaximized = window()->isMaximized();
    window()->showFullScreen();

    //todo屏蔽了全局信号
//    emit dApp->signalM->hideBottomToolbar(true);
//    emit dApp->signalM->hideExtensionPanel(true);
//    emit dApp->signalM->hideTopToolbar(true);
}

void LibSlideShowPanel::startSlideShow(const ViewInfo &vinfo)
{
    if (vinfo.paths.isEmpty()) {
        return;
    }
    m_vinfo = vinfo;

    //防止F5进入后重复进入mouse move触发鼠标出现
    m_isf5move = true;
    this->setCursor(Qt::BlankCursor);

    if (1 < vinfo.paths.length()) {
        slideshowbottombar->m_preButton->setEnabled(true);
        slideshowbottombar->m_nextButton->setEnabled(true);
        slideshowbottombar->m_playpauseButton->setEnabled(true);
        //todo屏蔽了全局信号
//        emit dApp->signalM->initSlideShowButton();
    } else {
        slideshowbottombar->m_preButton->setEnabled(false);
        slideshowbottombar->m_nextButton->setEnabled(false);
        slideshowbottombar->m_playpauseButton->setEnabled(false);
        //todo屏蔽了全局信号
//        emit dApp->signalM->updatePauseButton();
    }
    int number = QApplication::desktop()->screenNumber(this);
    if (number < 0) {
        number = 0;
    }
    int nParentWidth = QGuiApplication::screens().at(number)->geometry().width();
    int nParentHeight = QGuiApplication::screens().at(number)->geometry().height();
    slideshowbottombar->move((nParentWidth - slideshowbottombar->width()) / 2, nParentHeight);
    m_animation->startSlideShow(m_vinfo.path, m_vinfo.paths);
    auto actionlist = m_menu->actions();
    for (auto &action : actionlist) {
        if (action->property("MenuID").toInt() == IdPlayOrPause) {
            action->setText(tr(slideshowbottombar->m_playpauseButton->toolTip().toStdString().c_str()));
        }
    }
    //加入显示动画效果，以透明度0-1显示，动态加载，视觉效果掩盖左上角展开
    QPropertyAnimation *animation = new QPropertyAnimation(window(), "windowOpacity");
    animation->setDuration(50);
    animation->setEasingCurve(QEasingCurve::Linear);
    animation->setStartValue(0);
    animation->setEndValue(1);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
    m_hideCursorTid = startTimer(DELAY_HIDE_CURSOR_INTERVAL);
    showFullScreen();
}

void LibSlideShowPanel::onMenuItemClicked(QAction *action)
{
    const int id = action->property("MenuID").toInt();
    switch (id) {
    case IdStopslideshow:
        backToLastPanel();
        break;
    case IdPlayOrPause:
        slideshowbottombar->m_playpauseButton->clicked();
        action->setText(tr(slideshowbottombar->m_playpauseButton->toolTip().toStdString().c_str()));
        break;
    default:
        break;
    }
}

//void SlideShowPanel::onThemeChanged(ViewerThemeManager::AppTheme dark)
//{
//    if (dark == ViewerThemeManager::Dark) {
//        m_bgColor = DARK_BG_COLOR;
//    } else {
//        m_bgColor = LIGHT_BG_COLOR;
//    }
//    update();
//}

void LibSlideShowPanel::onSingleAnimationEnd()
{
    return ;
}

void LibSlideShowPanel::onESCKeyStopSlide()
{
    if (isVisible())
        backToLastPanel();
}

void LibSlideShowPanel::onShowPause()
{
    m_animation->pauseAndNext();
}

void LibSlideShowPanel::onShowContinue()
{
    m_animation->ifPauseAndContinue();
}

void LibSlideShowPanel::onShowPrevious()
{
    m_animation->playAndPre();
}

void LibSlideShowPanel::onShowNext()
{
    m_animation->playAndNext();
}

void LibSlideShowPanel::onCustomContextMenuRequested()
{
    this->setCursor(Qt::ArrowCursor);
    m_menu->popup(QCursor::pos());
}

void LibSlideShowPanel::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    if (m_isf5move) {
        m_isf5move = false;
    } else {
        this->setCursor(Qt::ArrowCursor);
    }

    if (window()->isFullScreen()) {
        QPoint pos = mapFromGlobal(QCursor::pos());
        // 处理程序界面的初始高度和全屏下幻灯片界面不一致导致底部工具栏位置错误
        int number = QApplication::desktop()->screenNumber(this);
        if (number < 0) {
            number = 0;
        }
        if (QGuiApplication::screens().at(number)->geometry().size().height() != height())
            return;
        if (height() - 20 < pos.y() && height() >= pos.y() && height() >= slideshowbottombar->y()) {
            QPropertyAnimation *animation = new QPropertyAnimation(slideshowbottombar, "pos");
            animation->setDuration(200);
            //animation->setEasingCurve(QEasingCurve::NCurveTypes);
            animation->setStartValue(QPoint((width() - slideshowbottombar->width()) / 2, slideshowbottombar->y()));
            animation->setEndValue(QPoint((width() - slideshowbottombar->width()) / 2, height() - slideshowbottombar->height() - 10));
            animation->start(QAbstractAnimation::DeleteWhenStopped);
            m_animation->update();
        } else if (height() - slideshowbottombar->height() - 10 > pos.y() && height() - slideshowbottombar->height() - 10 <= slideshowbottombar->y()) {
            QPropertyAnimation *animation = new QPropertyAnimation(slideshowbottombar, "pos");
            animation->setDuration(200);
            //animation->setEasingCurve(QEasingCurve::NCurveTypes);
            animation->setStartValue(QPoint((width() - slideshowbottombar->width()) / 2, slideshowbottombar->y()));
            animation->setEndValue(QPoint((width() - slideshowbottombar->width()) / 2, height()));
            animation->start(QAbstractAnimation::DeleteWhenStopped);
            m_animation->update();
        }
    }
}

void LibSlideShowPanel::timerEvent(QTimerEvent *event)
{
    //如果右键菜单存在则菜单不显示
    if (event->timerId() == m_hideCursorTid && qApp->modalWindow() == nullptr && (!m_menu || !m_menu->isVisible())) {
        this->setCursor(Qt::BlankCursor);
    }
    QWidget::timerEvent(event);
}

void LibSlideShowPanel::mouseDoubleClickEvent(QMouseEvent *e)
{
    //解决幻灯片无法使用双击退出问题bug67409
    if (e->button() == Qt::LeftButton) {
        backToLastPanel();
    }
    QWidget::mouseDoubleClickEvent(e);
}

