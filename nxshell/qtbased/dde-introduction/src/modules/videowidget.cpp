// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "videowidget.h"

static QDir ResourcesQDir()
{
    QDir videoPath(qApp->applicationDirPath());
    videoPath.setSorting(QDir::Name);
    bool temp = videoPath.cd("/usr/share/dde-introduction");//绝对路径
    qInfo()<<"return value = "<<temp;
    return videoPath;
}

VideoWidget::VideoWidget(bool autoPlay, QWidget *parent)
    : ModuleInterface(parent)
    , m_video(new dmr::PlayerWidget(this))
    , m_control(new DImageButton(this))
    , m_clip(new DClipEffectWidget(m_video))
    , m_btnAni(new QPropertyAnimation(m_control, "pos", this))
    , m_hideAni(new QPropertyAnimation(this))
    , m_leaveTimer(new QTimer(this))
    , m_pauseTimer(new QTimer(this))
    , m_background(new CoverPhoto(this))
{
    m_selectBtn->hide();

    m_leaveTimer->setSingleShot(true);
    m_leaveTimer->setInterval(1000);

    //这是什么效果,半透明吗?
    m_hideEffect = new QGraphicsOpacityEffect(m_control);
    m_hideEffect->setOpacity(1);
    m_control->setGraphicsEffect(m_hideEffect);

    //这是什么动画?按钮上下移动动画
    m_btnAni->setDuration(250);
    m_btnAni->setEasingCurve(QEasingCurve::InOutCubic);

    //这是什么动画?按钮隐藏动画,好像没什么动画效果啊?
    m_hideAni->setDuration(250);
    m_hideAni->setEasingCurve(QEasingCurve::InOutCubic);
    m_hideAni->setPropertyName("opacity");
    m_hideAni->setTargetObject(m_hideEffect);
    m_hideAni->setStartValue(1.0f);
    m_hideAni->setEndValue(0.0f);

    //Community : 社区版
    m_isCommunity = DSysInfo::isCommunityEdition();
    //社区版/专业版/个人版视频封面不同

    m_isCommunity ? m_strVideoCoverIcon = QString(":/resources/community_Moment.png") :
                    m_strVideoCoverIcon = QString(":/resources/demo_Moment.jpg");

    connect(m_hideAni, &QPropertyAnimation::finished, this, [=] { m_control->hide(); });

    connect(m_leaveTimer, &QTimer::timeout, this, [=] { m_hideAni->start(); });

    setObjectName("VideoWidget");

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(m_video, 0, Qt::AlignCenter);

    updateBigIcon();

    m_control->setFixedSize(48, 48);
    m_control->raise();

    connect(m_control, &DImageButton::clicked, this, &VideoWidget::onControlButtonClicked,
            Qt::QueuedConnection);
    connect(&m_video->engine(), &dmr::PlayerEngine::stateChanged, this,
            &VideoWidget::updateControlButton, Qt::QueuedConnection);

    //这是做什么??? zyf不知道
    autoPlay = !autoPlay;
    m_video->engine().setBackendProperty("pause-on-start", autoPlay ? "false" : "true");
    m_video->engine().setBackendProperty("panscan", 1.0);

    updateControlButton();
    setLayout(layout);

    m_background->setWindowFlags(Qt::FramelessWindowHint);
    m_background->raise();
    m_control->raise();
    m_load = false;
}

/*******************************************************************************
 1. @函数:    updateBigIcon
 2. @作者:
 3. @日期:    2020-12-08
 4. @说明:    更新第一次启动背景图
*******************************************************************************/
void VideoWidget::updateBigIcon()
{
    setFixedSize(700, 450);
    m_video->setFixedSize(this->size());
    if (m_background != nullptr) {
        QPixmap pixmap(m_strVideoCoverIcon);
        pixmap = pixmap.scaled(m_video->size(), Qt::IgnoreAspectRatio,
                               Qt::SmoothTransformation);
        m_background->setFixedSize(m_video->size());
        m_background->setPixmap(pixmap);
    }

    updateClip();
}

/*******************************************************************************
 1. @函数:    updateSmallIcon
 2. @作者:
 3. @日期:    2020-12-08
 4. @说明:    更新日常的背景图
*******************************************************************************/
void VideoWidget::updateSmallIcon()
{
    QSize size(549, 309);
    setFixedSize(size);
    m_video->setFixedSize(size);
    if (m_background != nullptr) {
        QPixmap pixmap(m_strVideoCoverIcon);
        pixmap = pixmap.scaled(size, Qt::IgnoreAspectRatio,
                               Qt::SmoothTransformation);
        m_background->setFixedSize(m_video->size());
        m_background->setPixmap(pixmap);
    }

    updateClip();
}

void VideoWidget::updateSelectBtnPos() {}


/*******************************************************************************
 1. @函数:    updateControlButton
 2. @作者:
 3. @日期:    2020-12-08
 4. @说明:    根据视频的播放状态更新播放按钮的效果图片
*******************************************************************************/
void VideoWidget::updateControlButton() //封面图和视频播放直接切换不处理不会有问题??? 封面图片会一直在
{
    const QPoint &p = rect().center() - m_control->rect().center();

    // switch (m_video->engine().state()) {
    if (m_video->engine().state() == dmr::PlayerEngine::Playing) {
        //播放时按钮背景图
        m_control->setNormalPic(":/resources/pause_normal.svg");
        m_control->setHoverPic(":/resources/pause_hover.svg");
        m_control->setPressPic(":/resources/pause_press.svg");
        m_btnAni->stop();
        m_btnAni->setStartValue(p);
        m_btnAni->setEndValue(QPoint(p.x(), height() - m_control->height() - 20));
        m_btnAni->start();
        //Add by ut001000 renfeixiang 2020-12-28 将删除背景图片的位置修改到deleteBackground函数中
//        if (m_background != nullptr) {
//            delete m_background;
//            m_background = nullptr;
//        }
    } else {
        //暂停时按钮背景图
        m_control->setNormalPic(":/resources/play_normal.svg");
        m_control->setHoverPic(":/resources/play_hover.svg");
        m_control->setPressPic(":/resources/play_press.svg");
        if (m_btnAni->startValue().toPoint() == p) {
            m_hideEffect->setOpacity(1);
            m_control->show();
            m_leaveTimer->stop();
            m_btnAni->stop();
            m_btnAni->setStartValue(m_control->pos());
            m_btnAni->setEndValue(p);
            m_btnAni->start();
        }
    }
}

/*******************************************************************************
 1. @函数:    onControlButtonClicked
 2. @作者:
 3. @日期:    2020-12-08
 4. @说明:    播放按钮点击响应函数
*******************************************************************************/
void VideoWidget::onControlButtonClicked()
{
    //点击播放视频时，取消选中框
    emit cancelCloseFrame();

    //加载视频
    if (!m_load) {
        qreal ratio = 1.0;
        QDir videoPath = ResourcesQDir();
        //修改回原有的固定使用视频名称,打各个版本包时将视频内容更换,名字不变
        QString file =  videoPath.path() + QString("/demo.mp4");

//        if (DSysInfo::uosEditionType() == DSysInfo::UosEdition::UosEnterprise ||
//            DSysInfo::uosEditionType() == DSysInfo::UosEdition::UosEnterpriseC ||
//            DSysInfo::uosEditionType() == DSysInfo::UosEdition::UosEuler) {
//            file += QString("/server.mp4");
//        } else if (DSysInfo::uosEditionType() == DSysInfo::UosEdition::UosCommunity) {
//            file += QString("/community.mp4");
//        } else {
//            file += QString("/professional.mp4");
//        }

        //添加视频路径到m_video
        m_video->engine().playlist().append(
            QUrl::fromLocalFile(qt_findAtNxFile(file, devicePixelRatioF(), &ratio)));
        //设置播放模式
        m_video->engine().playlist().setPlayMode(dmr::PlaylistModel::SingleLoop);
        m_load = true;
        //Add by ut001000 renfeixiang 2020-12-28 绑定删除背景函数，当视频解析结束，发送信号
        connect(&m_video->engine(), &dmr::PlayerEngine::videoSizeChanged, this , &VideoWidget::deleteBackground);
    }

    //问题：点击暂停也触发这
    QTimer::singleShot(30, this, SLOT(PauseResetOrPlay()));
}

void VideoWidget::PauseResetOrPlay() {
    m_video->engine().pauseResume();
    m_video->engine().play();
}

/*******************************************************************************
 1. @函数:    stop
 2. @作者:
 3. @日期:    2020-12-08
 4. @说明:    将当前动画暂停,下次播放从当前进度播放
*******************************************************************************/
void VideoWidget::stop()
{
    if (m_video->engine().state() == dmr::PlayerEngine::Playing) {
        m_video->engine().pauseResume();//什么作用???
        m_video->engine().play();//什么作用???
    }
}

/*******************************************************************************
 1. @函数:    updateInterface
 2. @作者:
 3. @日期:    2020-12-08
 4. @说明:    只是更新界面和背景大小
*******************************************************************************/
void VideoWidget::updateInterface(QSize size)
{
    setFixedSize(size);
    m_video->setFixedSize(size);
    if (m_background != nullptr) {
        QPixmap pixmap(m_strVideoCoverIcon);
        pixmap = pixmap.scaled(m_video->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        m_background->setFixedSize(m_video->size());
        m_background->setPixmap(pixmap);
    }

    updateClip();
}

/*******************************************************************************
 1. @函数:    showVideoControlButton
 2. @作者:
 3. @日期:    2020-12-08
 4. @说明:    显示播放按钮
*******************************************************************************/
void VideoWidget::showVideoControlButton()
{
    m_hideEffect->setOpacity(1);
    m_control->show();
    m_leaveTimer->stop();
}

/*******************************************************************************
 1. @函数:    deleteBackground
 2. @作者:    ut001000 任飞翔
 3. @日期:    2020-12-28
 4. @说明:    删除背景响应函数
*******************************************************************************/
void VideoWidget::deleteBackground()
{
    if(m_background != nullptr){
        qInfo() << "delete background";
        delete m_background;
        m_background = nullptr;
        disconnect(&m_video->engine(), &dmr::PlayerEngine::videoSizeChanged, this , &VideoWidget::deleteBackground);
    }
}

//鼠标进入事件,播放按钮显示,定时器停止
void VideoWidget::enterEvent(QEvent *e)
{
    ModuleInterface::enterEvent(e);

    m_hideEffect->setOpacity(1);
    m_control->show();
    m_leaveTimer->stop();
}

//鼠标离开事件,定时器开始,播放按钮准备隐藏
void VideoWidget::leaveEvent(QEvent *e)
{
    ModuleInterface::leaveEvent(e);

    if (m_video->engine().state() != dmr::PlayerEngine::Paused) {
        m_leaveTimer->start();
    }
}

//大小变化事件,大小变化时修改播放按钮的对应位置
void VideoWidget::resizeEvent(QResizeEvent *e)
{
    ModuleInterface::resizeEvent(e);

    m_control->move(rect().center() - m_control->rect().center());
}

void VideoWidget::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)
        onControlButtonClicked();
}

//不明白，如果屏蔽视频变白
void VideoWidget::updateClip()
{
    QPainterPath path;
    QRectF rectF;
    rectF.setTopLeft(rect().topLeft() - QPointF(2, 2));
    rectF.setBottomRight(rect().bottomRight());
    path.addRoundedRect(rectF, 5, 5);
    m_clip->setClipPath(path);
}
