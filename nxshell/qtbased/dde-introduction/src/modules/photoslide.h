// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PHOTOSLIDE_H
#define PHOTOSLIDE_H

#include "moduleinterface.h"

#include <QGraphicsOpacityEffect>
#include <QLabel>
#include <QPropertyAnimation>
#include <QWidget>
#include <QPainter>

#include <DLabel>
#include <DHiDPIHelper>
#include <DSysInfo>

#define UOSPROFESSIONAL_PHOTO_PATH  ":/resources/photo/UosProfessional"
#define UOSPROFESSIONAL_PHOTO_TOTAL 8
#define UOSENTERPRISE_PHOTO_PATH    ":/resources/photo/UosEnterprise"
#define UOSENTERPRISE_PHOTO_TOTAL   6
#define UOSEULER_PHOTO_PATH         ":/resources/photo/UosEuler"
#define UOSEULER_PHOTO_TOTAL        6
#define UOSPROFESSIONAL_LOONGARCH5000_PHOTO_PATH             ":/resources/photo/Mips/Loongarch5000Professional"
#define UOSPROFESSIONAL_LOONGARCH5000_PHOTO_TOTAL            6
#define UOSENTERPRISE_LOONGARCH5000_PHOTO_PATH               ":/resources/photo/Mips/Loongarch5000Enterprise"
#define UOSENTERPRISE_LOONGARCH5000_PHOTO_TOTAL              6

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

//图片轮播图类
class PhotoSlide : public ModuleInterface {
    Q_OBJECT
public:
    explicit PhotoSlide(QWidget* parent = nullptr);

    void updateBigIcon() override;
    void updateSmallIcon() override;
    void updateSelectBtnPos() override;

    void keyPressEvent(QKeyEvent *) override;

public slots:
    void start(bool disable_slide, bool disable_animation, int duration);
//    void stop();
    void setPhotoList(const QStringList& list);

private slots:
    // Update slide image while switching to next animation loop
    void updateSlideImage();

private:
    //初始化设置轮播图
    void initSetImage(DSysInfo::UosEdition imageType);

private:
    int                      slide_index_ {0};
    //不用
    int                      last_index_ {0};
    QStringList              m_photoList;
    //不用
    DLabel*                  container_label_   = nullptr;
    //不用
    QPropertyAnimation*      pos_animation_     = nullptr;
    //不用
    QGraphicsOpacityEffect*  opacity_effect_    = nullptr;
    //不用
    QPropertyAnimation*      opacity_animation_ = nullptr;
    //没用到
    QParallelAnimationGroup* animation_group_   = nullptr;
    //不用
    QVariantAnimation*       null_animation_    = nullptr;

    DLabel*                  m_lastLable        = nullptr;
    DLabel*                  m_currentLabel     = nullptr;
    QPropertyAnimation*      m_currentAni       = nullptr;
    QPropertyAnimation*      m_lastAni          = nullptr;
    //不同系统版本，用的轮播图不同，图片路径做区别
    QString m_strPhotoPath {QString()};
    int m_iHpotoTotal {0};
};

#endif  // PHOTOSLIDE_H
