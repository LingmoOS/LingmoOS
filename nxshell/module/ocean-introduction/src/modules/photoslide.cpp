// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "photoslide.h"

PhotoSlide::PhotoSlide(QWidget *parent)
    : ModuleInterface(parent)
{
    DSysInfo::UosEdition imageType = DSysInfo::uosEditionType();
    initSetImage(imageType);

    m_selectBtn->hide();

    /*container_label_ = new DLabel(this);

    pos_animation_ = new QPropertyAnimation(container_label_, "pos", this);

    opacity_effect_ = new QGraphicsOpacityEffect(container_label_);
    container_label_->setGraphicsEffect(opacity_effect_);
    opacity_animation_ = new QPropertyAnimation(opacity_effect_, "opacity", this);
    opacity_animation_->setKeyValueAt(0.0, 0.0);
    opacity_animation_->setKeyValueAt(0.1, 1.0);
    opacity_animation_->setKeyValueAt(0.9, 1.0);
    opacity_animation_->setKeyValueAt(1.0, 0.0);

    null_animation_ = new QVariantAnimation(this);

    animation_group_ = new QParallelAnimationGroup(this);
    animation_group_->setLoopCount(-1);

    connect(animation_group_, &QParallelAnimationGroup::currentLoopChanged, this, &PhotoSlide::updateSlideImage);*/

    //m_lastLable = new DLabel(this);
    //图片是加载在DLabel上面
    m_currentLabel = new DLabel(this);
    m_lastAni = new QPropertyAnimation(this);
    m_currentAni = new QPropertyAnimation(this);
    m_currentAni->setPropertyName("pos");
    m_lastAni->setPropertyName("pos");

    updateBigIcon();
}

void PhotoSlide::initSetImage(DSysInfo::UosEdition imageType) {
    if (imageType == DSysInfo::UosEdition::UosProfessional ||
            imageType == DSysInfo::UosEdition::UosCommunity) {
        m_strPhotoPath = QString(UOSPROFESSIONAL_PHOTO_PATH);
        m_iHpotoTotal  = UOSPROFESSIONAL_PHOTO_TOTAL;
        #if 0
        #if defined (ISLOONGARCH64)
            //龙芯5000专业版 misp架构有对应6张轮播图
            m_strPhotoPath = QString(UOSPROFESSIONAL_LOONGARCH5000_PHOTO_PATH);
            m_iHpotoTotal  = UOSPROFESSIONAL_LOONGARCH5000_PHOTO_TOTAL;
        #else
            m_strPhotoPath = QString(UOSPROFESSIONAL_PHOTO_PATH);
            m_iHpotoTotal  = UOSPROFESSIONAL_PHOTO_TOTAL;
        #endif
        #endif
    } else if (imageType == DSysInfo::UosEdition::UosEnterprise ||
               imageType == DSysInfo::UosEdition::UosEnterpriseC ) {
        #if defined (ISLOONGARCH64)
            //龙芯5000服务器企业版 misp架构对应6张轮播图
            m_strPhotoPath = QString(UOSENTERPRISE_LOONGARCH5000_PHOTO_PATH);
            m_iHpotoTotal  = UOSENTERPRISE_LOONGARCH5000_PHOTO_TOTAL;
        #else
        //服务器专业版/行业版，轮播图6张
        m_strPhotoPath = QString(UOSENTERPRISE_PHOTO_PATH);
        m_iHpotoTotal  = UOSENTERPRISE_PHOTO_TOTAL;
        #endif
    } else if (imageType == DSysInfo::UosEdition::UosEuler) {
        //欧拉版,轮播图8张
        m_strPhotoPath = QString(UOSEULER_PHOTO_PATH);
        m_iHpotoTotal  = UOSEULER_PHOTO_TOTAL;
    } else {
        //考虑到如果在新类型系统上跑应用，默认使用8张的专业版轮播图
        m_strPhotoPath = QString(UOSPROFESSIONAL_PHOTO_PATH);
        m_iHpotoTotal  = UOSPROFESSIONAL_PHOTO_TOTAL;
    }
}

/*******************************************************************************
 1. @函数:    updateBigIcon
 2. @作者:
 3. @日期:    2020-12-08
 4. @说明:    更新第一次启动轮播图
*******************************************************************************/
void PhotoSlide::updateBigIcon()
{
    const QSize size(700, 450);
    setFixedSize(size);
    //container_label_->setFixedSize(size);
    m_currentLabel->setFixedSize(size);

    QStringList list;
    for (int i = 1; i <= m_iHpotoTotal; i++) {
        list << QString("%1/slide_big/0%2.jpg").arg(m_strPhotoPath).arg(i);
    }

    setPhotoList(list);

    //pos_animation_->setKeyValueAt(0.0, QPoint(-50, 0));
    //pos_animation_->setKeyValueAt(0.1, QPoint(0, 0));
    //pos_animation_->setKeyValueAt(0.9, QPoint(0, 0));
    //pos_animation_->setKeyValueAt(1.0, QPoint(50, 0));
}

/*******************************************************************************
 1. @函数:    updateSmallIcon
 2. @作者:
 3. @日期:    2020-12-08
 4. @说明:    更新日常的轮播图
*******************************************************************************/
void PhotoSlide::updateSmallIcon()
{
    const QSize size(549, 309);
    setFixedSize(size);
    //container_label_->setFixedSize(size);
    m_currentLabel->setFixedSize(size);

    QStringList list;
    for (int i = 1; i <= m_iHpotoTotal; i++) {
        list << QString("%1/slide_small/0%2.jpg").arg(m_strPhotoPath).arg(i);
    }

    setPhotoList(list);

    //pos_animation_->setKeyValueAt(0.0, QPoint(-50, 0));
    //pos_animation_->setKeyValueAt(0.1, QPoint(15, 0));
    //pos_animation_->setKeyValueAt(0.9, QPoint(15, 0));
    //pos_animation_->setKeyValueAt(1.0, QPoint(50, 0));
}

void PhotoSlide::updateSelectBtnPos() {}

void PhotoSlide::keyPressEvent(QKeyEvent *e)
{
    Q_UNUSED(e);
//    QWidget::keyPressEvent(e);
}

//第二参数不用，可以删除
void PhotoSlide::start(bool disable_slide, bool disable_animation, int duration)
{
    Q_UNUSED(disable_animation);
    Q_UNUSED(duration);
    // Read the first slide image.
    slide_index_ = 0;
    updateSlideImage();

    if (disable_slide) {
        qDebug() << "slide disabled";
        return;
    }

    /*animation_group_->clear();
    if (disable_animation) {
        qDebug() << "slide animation disabled";
        null_animation_->setDuration(duration);
        animation_group_->addAnimation(null_animation_);
    }
    else {
        pos_animation_->setDuration(duration);
        animation_group_->addAnimation(pos_animation_);
        opacity_animation_->setDuration(duration);
        animation_group_->addAnimation(opacity_animation_);
    }
    animation_group_->start();*/
    QTimer *time = new QTimer;
    connect(time, &QTimer::timeout, this, &PhotoSlide::updateSlideImage);
    time->start(2000);
}

//void PhotoSlide::stop()
//{
//    animation_group_->stop();
//}

//保存照片的list
void PhotoSlide::setPhotoList(const QStringList &list)
{
    //如果照片名字是一样的？？？内容更换了，会不会return???
    if (m_photoList == list) return;

    m_photoList = list;
}


/*******************************************************************************
 1. @函数:    updateSlideImage
 2. @作者:
 3. @日期:    2020-12-09
 4. @说明:    更新轮播图的图片
*******************************************************************************/
void PhotoSlide::updateSlideImage()
{
    if (m_currentAni->state() == QPropertyAnimation::Running) {
        return;
    }

    const QString filepath(m_photoList[slide_index_]);
    if(m_lastLable) {
        m_lastLable->deleteLater();
    }
    m_lastLable = m_currentLabel;
    m_currentLabel = new DLabel(this);

    if (QFile::exists(filepath)) {
        QPixmap pixmap = DHiDPIHelper::loadNxPixmap(filepath);
        m_currentLabel->setPixmap(pixmap);
        m_currentLabel->setScaledContents(true);
    }
    else {
        qWarning() << "slide file not found:" << filepath;
    }

    m_currentLabel->show();
    m_currentAni->setTargetObject(m_currentLabel);
    m_lastAni->setTargetObject(m_lastLable);

    m_lastAni->setDuration(300);
    m_lastAni->setEasingCurve(QEasingCurve::InOutCubic);
    m_lastAni->setStartValue(m_lastLable->rect().topLeft());
    m_lastAni->setEndValue(QPoint(m_lastLable->x() - m_lastLable->width(), 0));

    m_currentAni->setDuration(300);
    m_currentAni->setEasingCurve(QEasingCurve::InOutCubic);
    m_currentAni->setStartValue(QPoint(m_lastLable->rect().topRight()));
    m_currentAni->setEndValue(QPoint(0, 0));

    m_currentAni->start();
    m_lastAni->start();

    slide_index_ = (slide_index_ + 1) % m_photoList.length();
}

