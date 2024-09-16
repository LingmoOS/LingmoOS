// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "SlidePlayWidget.h"
#include "Utils.h"

#include <DPlatformWindowHandle>
#include <DWidget>

#include <QHBoxLayout>
#include <QDBusInterface>
#include <QDBusConnection>
#include <DGuiApplicationHelper>

SlidePlayWidget::SlidePlayWidget(QWidget *parent) : DFloatingWidget(parent)
{
    initControl();
}

void SlidePlayWidget::initControl()
{
    setBlurBackgroundEnabled(true);
    setFramRadius(18);

    m_timer.setInterval(3000);
    m_timer.start();
    connect(&m_timer, &QTimer::timeout, this, &SlidePlayWidget::onTimerout);

    QHBoxLayout *playout = new QHBoxLayout;
    setContentsMargins(0, 0, 0, 0);
    playout->setContentsMargins(10, 10, 10, 10);
    playout->setSpacing(10);

    m_preBtn  = createBtn(QString("previous_normal"));
    DIconButton *pbtnplay = createBtn(QString("play_normal"));
    m_nextBtn = createBtn(QString("next_normal"));
    DIconButton *pbtnexit = createBtn(QString("exit_normal"));
    m_playBtn = pbtnplay;

    playout->addWidget(m_preBtn);
    playout->addWidget(pbtnplay);
    playout->addWidget(m_nextBtn);
    playout->addWidget(pbtnexit);

    DWidget *pwidget = new DWidget;
    pwidget->setLayout(playout);
    this->setWidget(pwidget);

    connect(m_preBtn,  &DIconButton::clicked, this, &SlidePlayWidget::onPreClicked);
    connect(pbtnplay, &DIconButton::clicked, this, &SlidePlayWidget::onPlayClicked);
    connect(m_nextBtn, &DIconButton::clicked, this, &SlidePlayWidget::onNextClicked);
    connect(pbtnexit, &DIconButton::clicked, this, &SlidePlayWidget::onExitClicked);

    setFocusPolicy(Qt::NoFocus);
    m_preBtn->setFocusPolicy(Qt::NoFocus);
    pbtnplay->setFocusPolicy(Qt::NoFocus);
    m_nextBtn->setFocusPolicy(Qt::NoFocus);
    pbtnexit->setFocusPolicy(Qt::NoFocus);
#ifdef DTKWIDGET_CLASS_DSizeMode
    if (DGuiApplicationHelper::instance()->sizeMode() == DGuiApplicationHelper::NormalMode) {
        m_preBtn->setFixedSize(50, 50);
        m_preBtn->setIconSize(QSize(36, 36));
        m_playBtn->setFixedSize(50, 50);
        m_playBtn->setIconSize(QSize(36, 36));
        m_nextBtn->setFixedSize(50, 50);
        m_nextBtn->setIconSize(QSize(36, 36));
        pbtnexit->setFixedSize(50, 50);
        pbtnexit->setIconSize(QSize(36, 36));
    } else {
        m_preBtn->setFixedSize(36, 36);
        m_preBtn->setIconSize(QSize(24, 24));
        m_playBtn->setFixedSize(36, 36);
        m_playBtn->setIconSize(QSize(24, 24));
        m_nextBtn->setFixedSize(36, 36);
        m_nextBtn->setIconSize(QSize(24, 24));
        pbtnexit->setFixedSize(36, 36);
        pbtnexit->setIconSize(QSize(24, 24));
    }
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [=](DGuiApplicationHelper::SizeMode sizeMode) {
        if (sizeMode == DGuiApplicationHelper::NormalMode) {
            m_preBtn->setFixedSize(50, 50);
            m_preBtn->setIconSize(QSize(36, 36));
            m_playBtn->setFixedSize(50, 50);
            m_playBtn->setIconSize(QSize(36, 36));
            m_nextBtn->setFixedSize(50, 50);
            m_nextBtn->setIconSize(QSize(36, 36));
            pbtnexit->setFixedSize(50, 50);
            pbtnexit->setIconSize(QSize(36, 36));
        } else {
            m_preBtn->setFixedSize(36, 36);
            m_preBtn->setIconSize(QSize(24, 24));
            m_playBtn->setFixedSize(36, 36);
            m_playBtn->setIconSize(QSize(24, 24));
            m_nextBtn->setFixedSize(36, 36);
            m_nextBtn->setIconSize(QSize(24, 24));
            pbtnexit->setFixedSize(36, 36);
            pbtnexit->setIconSize(QSize(24, 24));
        }
        if(isVisible()) {
            hide();
            showControl();
        }
    });
#endif
}

void SlidePlayWidget::showControl()
{
    m_timer.start();
    this->show();
}

void SlidePlayWidget::onTimerout()
{
    this->hide();
}

void SlidePlayWidget::enterEvent(QEvent *event)
{
    m_timer.stop();
    DFloatingWidget::enterEvent(event);
}

void SlidePlayWidget::leaveEvent(QEvent *event)
{
    m_timer.start();
    DFloatingWidget::leaveEvent(event);
}

DIconButton *SlidePlayWidget::createBtn(const QString &strname)
{
    DIconButton *btn = new  DIconButton(this);
    btn->setObjectName(strname);
    btn->setFixedSize(50, 50);
    btn->setIcon(/*PF::getIcon*/QIcon::fromTheme(QString("dr_") + strname));
    btn->setIconSize(QSize(36, 36));
    btn->setWindowFlag(Qt::FramelessWindowHint);
    return  btn;
}

void SlidePlayWidget::setPlayStatus(bool play)
{
    m_autoPlay = play;
    playStatusChanged();
}

void SlidePlayWidget::updateProcess(int cur, int total)
{
    /* 幻灯片放映时
     * cur:当前页、total总页数
     * 当cur:0, total:3
     * 已到第一页时，再次点击上一页按钮，cur=-1,则设置preBtn为disable
     *
     * 当cur:2, total:3
     * 已到最后一页时，再次点击下一页按钮，cur=3,则设置nextBtn为disable
     */
    if(cur <= -1) {
        if(m_preBtn->isEnabled())
            Notify(tr("It is the first page"));
        m_preBtn->setEnabled(false);
        m_nextBtn->setEnabled(true);
    } else if(cur >=  total) {
        if(m_nextBtn->isEnabled())
            Notify(tr("It is the last page"));
        m_preBtn->setEnabled(true);
        m_nextBtn->setEnabled(false);
        setPlayStatus(false);
    } else {
        m_preBtn->setEnabled(true);
        m_nextBtn->setEnabled(true);
    }
}

void SlidePlayWidget::Notify(const QString &text)
{
    if(nullptr == m_dbusNotify) {
        m_dbusNotify = new QDBusInterface("com.deepin.dde.Notification",
                                          "/com/deepin/dde/Notification",
                                          "com.deepin.dde.Notification",
                                          QDBusConnection::sessionBus(),
                                          this);
    }
    //初始化Notify 七个参数
    QString appname("deepin-reader");
    uint replaces_id = 0;
    QString appicon("deepin-reader");
    QString title = "";
    QString body = text;
    QStringList actionlist;
    QVariantMap hints;
    int timeout = 3000;
    m_dbusNotify->call("Notify", appname, replaces_id, appicon, title, body, actionlist, hints, timeout);
}

bool SlidePlayWidget::getPlayStatus()
{
    return m_autoPlay;
}

void SlidePlayWidget::onPreClicked()
{
    emit signalPreBtnClicked();
}

void SlidePlayWidget::onPlayClicked()
{
    m_autoPlay = !m_autoPlay;
    playStatusChanged();
}

void SlidePlayWidget::onNextClicked()
{
    emit signalNextBtnClicked();
}

void SlidePlayWidget::onExitClicked()
{
    emit signalExitBtnClicked();
}

void SlidePlayWidget::playStatusChanged()
{
    if (m_autoPlay) {
        m_playBtn->setIcon(QIcon::fromTheme(QString("dr_") + "suspend_normal"));
    } else {
        m_playBtn->setIcon(QIcon::fromTheme(QString("dr_") + "play_normal"));
    }
    emit signalPlayBtnClicked();
}

void SlidePlayWidget::mousePressEvent(QMouseEvent *)
{
    //Nottodo
}
