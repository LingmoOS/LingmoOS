// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "takephotosettingareawidget.h"
#include "ac-deepin-camera-define.h"
#include "circlepushbutton.h"
#include "filterpreviewbutton.h"

#include <QColor>
#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include <QGraphicsOpacityEffect>
#include <QDateTime>
#include <QTimer>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QScrollBar>

#define ANIMATION_DURATION 200
#define ANIMATION_FOLD_DURATION 200
#define ANIMATION_ROTATE_DURATION 100
#define ANIMATION_FILTER_DURATION 170
#define SLIDER_ANIMATION_DURATION 50
#define EXPOSURE_SLIDER_HEIGHT 192

#define LEFT_MARGIN_PIX 10

#define EASING_OVERSHOOT 0.82079

takePhotoSettingAreaWidget::takePhotoSettingAreaWidget(QWidget *parent) : QWidget(parent)
    , m_btnHeightOffset(20)//暂时间隔设定为20,需确定后修改
    , m_threeBtnOffset(6)
    , m_foldBtn(nullptr)
    , m_unfoldBtn(nullptr)
    , m_flashlightFoldBtn(nullptr)
    , m_flashlightUnfoldBtn(nullptr)
    , m_flashlightOnBtn(nullptr)
    , m_flashlightOffBtn(nullptr)
    , m_delayFoldBtn(nullptr)
    , m_delayUnfoldBtn(nullptr)
    , m_noDelayBtn(nullptr)
    , m_delay3SecondBtn(nullptr)
    , m_delay6SecondBtn(nullptr)
    , m_exposureSlider(nullptr)
    , m_bPhoto(true)
    , m_filterType(filter_Normal)
{
    m_delayGroupDisplay = false;
    m_flashGroupDisplay = false;
    m_bPhotoToVideState = false;
    m_isBtnsFold = true;
    m_buttonGroupColor.setRgb(0, 0, 0, 255 * 0.4);
}

void takePhotoSettingAreaWidget::initButtons()
{
    this->setObjectName(LEFT_BTNS_BOX);
    this->setAccessibleName(LEFT_BTNS_BOX);

    m_unfoldBtn = new circlePushButton(this);
    m_unfoldBtn->setPixmap(":/images/camera/unfold.svg", ":/images/camera/unfold.svg", ":/images/camera/unfold.svg");
    m_unfoldBtn->setDisableSelect(true);
    m_unfoldBtn->setObjectName(UNFOLD_BTN);
    m_unfoldBtn->setAccessibleName(UNFOLD_BTN);

    // 闪光灯
    m_flashlightFoldBtn = new circlePushButton(this);
    m_flashlightFoldBtn->setPixmap(":/images/camera/flashlight.svg", ":/images/camera/flashlight-hover.svg", ":/images/camera/flashlight-press.svg");
    m_flashlightFoldBtn->setDisableSelect(true);
    m_flashlightFoldBtn->setObjectName(FLASHLITE_FOLD_BTN);
    m_flashlightFoldBtn->setAccessibleName(FLASHLITE_FOLD_BTN);

    m_flashlightUnfoldBtn = new circlePushButton(this);
    m_flashlightUnfoldBtn->setPixmap(":/images/camera/close-flashlight.svg", ":/images/camera/close-flashlight-hover.svg", ":/images/camera/close-flashlight-press.svg");
    m_flashlightUnfoldBtn->setDisableSelect(true);
    m_flashlightUnfoldBtn->setObjectName(FLASHLITE_UNFOLD_BTN);
    m_flashlightUnfoldBtn->setAccessibleName(FLASHLITE_UNFOLD_BTN);
    m_flashlightUnfoldBtn->setToolTip(tr("Flashlight"));
    m_flashlightUnfoldBtn->setFocusPolicy(Qt::NoFocus);

    m_flashlightOnBtn = new circlePushButton(this);
    m_flashlightOnBtn->setPixmap(":/images/camera/flashlight.svg", ":/images/camera/flashlight-hover.svg", ":/images/camera/flashlight-press.svg");
    m_flashlightOnBtn->setObjectName(FLASHLITE_ON_BTN);
    m_flashlightOnBtn->setAccessibleName(FLASHLITE_ON_BTN);
    m_flashlightOnBtn->setToolTip(tr("On"));
    m_flashlightOnBtn->setFocusPolicy(Qt::NoFocus);

    m_flashlightOffBtn = new circlePushButton(this);
    m_flashlightOffBtn->setPixmap(":/images/camera/close-flashlight.svg", ":/images/camera/close-flashlight-hover.svg", ":/images/camera/close-flashlight-press.svg");
    m_flashlightOffBtn->setObjectName(FLASHLITE_OFF_BTN);
    m_flashlightOffBtn->setAccessibleName(FLASHLITE_OFF_BTN);
    m_flashlightOffBtn->setToolTip(tr("Off"));
    m_flashlightOffBtn->setFocusPolicy(Qt::NoFocus);

    // 延迟拍摄
    m_delayFoldBtn = new circlePushButton(this);
    m_delayFoldBtn->setPixmap(":/images/camera/delay.svg", ":/images/camera/delay-hover.svg", ":/images/camera/delay-press.svg");
    m_delayFoldBtn->setDisableSelect(true);
    m_delayFoldBtn->setObjectName(DELAY_FOLD_BTN);
    m_delayFoldBtn->setAccessibleName(DELAY_FOLD_BTN);

    m_delayUnfoldBtn = new circlePushButton(this);
    m_delayUnfoldBtn->setPixmap(":/images/camera/delay.svg", ":/images/camera/delay-hover.svg", ":/images/camera/delay-press.svg");
    m_delayUnfoldBtn->setDisableSelect(true);
    m_delayUnfoldBtn->setObjectName(DELAY_FOLD_BTN);
    m_delayUnfoldBtn->setAccessibleName(DELAY_FOLD_BTN);
    m_delayUnfoldBtn->setToolTip(tr("Delay capture"));
    m_delayUnfoldBtn->setFocusPolicy(Qt::NoFocus);

    m_noDelayBtn = new circlePushButton(this);
    m_noDelayBtn->setPixmap(":/images/camera/delay.svg", ":/images/camera/delay-hover.svg", ":/images/camera/delay-press.svg");
    m_noDelayBtn->setObjectName(DELAY_UNFOLD_BTN);
    m_noDelayBtn->setAccessibleName(DELAY_UNFOLD_BTN);
    m_noDelayBtn->setToolTip(tr("None"));
    m_noDelayBtn->setFocusPolicy(Qt::NoFocus);

    m_delay3SecondBtn = new circlePushButton(this);
    m_delay3SecondBtn->setPixmap(":/images/camera/delay3s.svg", ":/images/camera/dalay-3s-hover.svg", ":/images/camera/delay-3s-press.svg");
    m_delay3SecondBtn->setObjectName(DELAY_3S_BTN);
    m_delay3SecondBtn->setAccessibleName(DELAY_3S_BTN);
    m_delay3SecondBtn->setFocusPolicy(Qt::NoFocus);
    m_delay3SecondBtn->setToolTip("3s");

    m_delay6SecondBtn = new circlePushButton(this);
    m_delay6SecondBtn->setPixmap(":/images/camera/delay6S.svg", ":/images/camera/delay-6S-hover.svg", ":/images/camera/delay-6S-press.svg");
    m_delay6SecondBtn->setObjectName(DELAY_6S_BTN);
    m_delay6SecondBtn->setAccessibleName(DELAY_6S_BTN);
    m_delay6SecondBtn->setFocusPolicy(Qt::NoFocus);
    m_delay6SecondBtn->setToolTip("6s");

    // 滤镜
    m_filtersFoldBtn = new circlePushButton(this);
    m_filtersFoldBtn->setPixmap(":/images/camera/filters.svg", ":/images/camera/filters-hover.svg", ":/images/camera/filters-press.svg");
    m_filtersFoldBtn->setDisableSelect(true);
    m_filtersFoldBtn->setObjectName(FILTERS_FOLD_BTN);
    m_filtersFoldBtn->setAccessibleName(FILTERS_FOLD_BTN);
    m_filtersFoldBtn->setToolTip(tr("Filters"));

    m_filtersUnfoldBtn = new circlePushButton(this);
    m_filtersUnfoldBtn->setPixmap(":/images/camera/filters.svg", ":/images/camera/filters-hover.svg", ":/images/camera/filters-press.svg");
    m_filtersUnfoldBtn->setDisableSelect(true);
    m_filtersUnfoldBtn->setObjectName(FILTERS_UNFOLD_BTN);
    m_filtersUnfoldBtn->setAccessibleName(FILTERS_UNFOLD_BTN);
    m_filtersUnfoldBtn->setToolTip(tr("Filters"));
    m_filtersUnfoldBtn->setFocusPolicy(Qt::NoFocus);

    //滤镜滚动条窗口
    m_scrollAreaWidget = new QWidget(this);
    m_scrollAreaWidget->setAttribute(Qt::WA_TranslucentBackground, true);
    QVBoxLayout *scrollLayout = new QVBoxLayout(m_scrollAreaWidget);
    scrollLayout->setContentsMargins(0, 0, 0, 0);
    scrollLayout->setSpacing(2);
    m_scrollAreaWidget->setLayout(scrollLayout);
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setWidget(m_scrollAreaWidget);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVisible(false);

    filterPreviewButton* filterBtn = nullptr;
    for (int i = 0; i < efilterType::filter_Count; i++) {
        filterBtn = new filterPreviewButton(nullptr, static_cast<efilterType>(i));
        filterBtn->setToolTip(filterPreviewButton::filterName(static_cast<efilterType>(i)));
        filterBtn->setFocusPolicy(Qt::NoFocus);
        filterBtn->show();
        filterBtn->setVisible(true);
        scrollLayout->addWidget(filterBtn);
        m_filterPreviewBtnList.push_back(filterBtn);
    }

    m_scrollHeight = (m_filtersFoldBtn->height() + m_threeBtnOffset * 2) * 8;//默认显示8个
    m_scrollAreaWidget->resize(filterBtn->width(), (m_filtersFoldBtn->height() + m_threeBtnOffset * 2) * efilterType::filter_Count);
    m_scrollArea->resize(m_scrollAreaWidget->width(), m_scrollHeight);

    m_filtersCloseBtn = new circlePushButton(this);
    m_filtersCloseBtn->setPixmap(":/images/camera/filters-close.svg", ":/images/camera/filters-close-hover.svg", ":/images/camera/filters-close-press.svg");
    m_filtersCloseBtn->setDisableSelect(true);
    m_filtersCloseBtn->setObjectName(FILTERS_CLOSE_BTN);
    m_filtersCloseBtn->setAccessibleName(FILTERS_CLOSE_BTN);
    //m_filtersCloseBtn->setToolTip(tr("Filters Close"));

    m_exposureBtn = new circlePushButton(this);
    m_exposureBtn->setPixmap(":/images/camera/exposure.svg", ":/images/camera/exposure-hover.svg", ":/images/camera/exposure-press.svg");
    m_exposureBtn->setObjectName(EXPOSURE_BTN);
    m_exposureBtn->setAccessibleName(EXPOSURE_BTN);
    m_exposureBtn->setToolTip(tr("Exposure"));
    m_exposureBtn->setFocusPolicy(Qt::NoFocus);

    m_foldBtn = new circlePushButton(this);
    m_foldBtn->setDisableSelect(true);
    m_foldBtn->setPixmap(":/images/camera/fold.svg", ":/images/camera/fold.svg", ":/images/camera/fold.svg");
    m_foldBtn->setObjectName(FOLD_BTN);
    m_foldBtn->setAccessibleName(FOLD_BTN);
    m_foldBtn->setFocusPolicy(Qt::NoFocus);

    m_exposureSlider = new ExposureSlider((QWidget *)this->parent());
    m_exposureSlider->setObjectName(EXPOSURE_SLIDER);
    m_exposureSlider->setAccessibleName(EXPOSURE_SLIDER);
    m_exposureSlider->hide();

    hideAll();
}

void takePhotoSettingAreaWidget::initLayout()
{
    QPoint pos = this->pos();
    int btnHeight = m_filtersUnfoldBtn->height();

    m_filtersUnfoldBtn->move(pos.x(), pos.y() + (btnHeight + m_btnHeightOffset) * 2);
}

void takePhotoSettingAreaWidget::init()
{
    initButtons();
    initLayout();

    connect(m_unfoldBtn, &QPushButton::clicked, this, &takePhotoSettingAreaWidget::unfoldBtnClicked);
    connect(m_foldBtn, &QPushButton::clicked, this, &takePhotoSettingAreaWidget::foldBtnClicked);

    connect(m_flashlightUnfoldBtn, &QPushButton::clicked, this, &takePhotoSettingAreaWidget::flashlightUnfoldBtnClicked);
    connect(m_flashlightFoldBtn, &QPushButton::clicked, this, &takePhotoSettingAreaWidget::flashlightFoldBtnClicked);

    connect(m_delayUnfoldBtn, &QPushButton::clicked, this, &takePhotoSettingAreaWidget::delayUnfoldBtnClicked);
    connect(m_delayFoldBtn, &QPushButton::clicked, this, &takePhotoSettingAreaWidget::delayfoldBtnClicked);

    connect(m_filtersUnfoldBtn, &QPushButton::clicked, this, &takePhotoSettingAreaWidget::filtersUnfoldBtnClicked);
    connect(m_filtersFoldBtn, &QPushButton::clicked, this, &takePhotoSettingAreaWidget::filtersFoldBtnClicked);

    connect(m_exposureBtn, &QPushButton::clicked, this, &takePhotoSettingAreaWidget::exposureBtnClicked);

    connect(m_noDelayBtn, &QPushButton::clicked, this, &takePhotoSettingAreaWidget::onDelayBtnsClicked);
    connect(m_delay3SecondBtn, &QPushButton::clicked, this, &takePhotoSettingAreaWidget::onDelayBtnsClicked);
    connect(m_delay6SecondBtn, &QPushButton::clicked, this, &takePhotoSettingAreaWidget::onDelayBtnsClicked);

    connect(m_noDelayBtn, &circlePushButton::clicked, this, &takePhotoSettingAreaWidget::onDelayBtnsClicked);
    connect(m_delay3SecondBtn, &circlePushButton::clicked, this, &takePhotoSettingAreaWidget::onDelayBtnsClicked);
    connect(m_delay6SecondBtn, &circlePushButton::clicked, this, &takePhotoSettingAreaWidget::onDelayBtnsClicked);

    connect(m_flashlightOnBtn, &QPushButton::clicked, this, &takePhotoSettingAreaWidget::onFlashlightBtnsClicked);
    connect(m_flashlightOffBtn, &QPushButton::clicked, this, &takePhotoSettingAreaWidget::onFlashlightBtnsClicked);

    connect(m_flashlightOnBtn, &circlePushButton::clicked, this, &takePhotoSettingAreaWidget::onFlashlightBtnsClicked);
    connect(m_flashlightOffBtn, &circlePushButton::clicked, this, &takePhotoSettingAreaWidget::onFlashlightBtnsClicked);

    for (auto btn : m_filterPreviewBtnList) {
        connect(btn, &filterPreviewButton::clicked, this, &takePhotoSettingAreaWidget::onFilterBtnsClicked);
    }

    connect(m_filtersCloseBtn, &QPushButton::clicked, this, &takePhotoSettingAreaWidget::filtersFoldBtnClicked);

    connect(m_exposureSlider, &ExposureSlider::valueChanged, this, &takePhotoSettingAreaWidget::onExposureValueChanged);
    connect(m_exposureSlider, &ExposureSlider::valueChanged, this, &takePhotoSettingAreaWidget::sigExposureChanged);

    //使用Enter键收起曝光条
    connect(m_exposureSlider, &ExposureSlider::enterBtnClicked, this, &takePhotoSettingAreaWidget::exposureBtnClicked);

    setFixedSize(QSize(m_unfoldBtn->width(), m_unfoldBtn->height()));
    m_unfoldBtn->setVisible(true);
}

void takePhotoSettingAreaWidget::showFold(bool bShow, bool isShortcut)
{
    Q_UNUSED(bShow);

    circlePushBtnList btnList;
    btnList.push_back(m_flashlightUnfoldBtn);
    btnList.push_back(m_delayUnfoldBtn);
    btnList.push_back(m_filtersUnfoldBtn);
    btnList.push_back(m_exposureBtn);
    btnList.push_back(m_foldBtn);

    // 以哪个按钮为原点来折叠动画
    circlePushButton* originBtn = m_filtersUnfoldBtn;
    if (!m_bPhoto)
        originBtn = m_delayUnfoldBtn;

    //位移动画
    QPoint endPos = QPoint(0, (originBtn->height() + m_btnHeightOffset) * 2); //结束位置固定为中间按钮
    if (!m_bPhoto)
        endPos = QPoint(0, 0);

    QParallelAnimationGroup *pPosGroup = new QParallelAnimationGroup(this);
    int index = 0;
    for (auto btn : btnList) {
        if (!m_bPhoto) {
            if (btn == m_flashlightUnfoldBtn
                    || btn == m_filtersUnfoldBtn
                    || btn == m_exposureBtn)
                continue;
        }

        QPropertyAnimation *opacity = new QPropertyAnimation(btn, "opacity", this);
        opacity->setDuration(ANIMATION_FOLD_DURATION);
        opacity->setStartValue(102);
        opacity->setEndValue(0);
        pPosGroup->addAnimation(opacity);

        QPropertyAnimation *iconOpacity = new QPropertyAnimation(btn, "iconopacity", this);
        iconOpacity->setDuration(ANIMATION_FOLD_DURATION);
        iconOpacity->setStartValue(1);
        iconOpacity->setEndValue(0);
        pPosGroup->addAnimation(iconOpacity);

        if (btn == originBtn) {
            originBtn->move(0,(originBtn->height() + m_btnHeightOffset) * index++);
            continue;
        }

        QPropertyAnimation* position = new QPropertyAnimation(btn, "pos", this);
        position->setDuration(ANIMATION_FOLD_DURATION);
        if (btn == m_foldBtn)
            //折叠按钮与上一按钮间距比普通间距多10px
            position->setStartValue(QPoint(0, 10 + (originBtn->height() + m_btnHeightOffset) * index++));
        else
            position->setStartValue(QPoint(0, (originBtn->height() + m_btnHeightOffset) * index++));
        position->setEndValue(endPos);
        pPosGroup->addAnimation(position);
    }

    pPosGroup->start(QAbstractAnimation::DeleteWhenStopped);

    closeAllGroup();

    //展开按钮旋转动画
    QPropertyAnimation * opacity = new QPropertyAnimation(m_unfoldBtn, "opacity", this);
    opacity->setStartValue(0);
    opacity->setEndValue(102);
    opacity->setDuration(ANIMATION_ROTATE_DURATION);

    QPropertyAnimation *rotate = new QPropertyAnimation(m_unfoldBtn, "rotate", this);
    rotate->setStartValue(90);
    rotate->setEndValue(0);
    rotate->setDuration(ANIMATION_ROTATE_DURATION);

    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
    group->addAnimation(opacity);
    group->addAnimation(rotate);

    connect(group, &QParallelAnimationGroup::finished, this, [=](){
        m_isBtnsFold = true;
        if (isShortcut)
            m_unfoldBtn->setFocus();
    });

    connect(pPosGroup, &QParallelAnimationGroup::finished, [=](){
        m_foldBtn->setVisible(false);
        m_flashlightUnfoldBtn->setVisible(false);
        m_delayUnfoldBtn->setVisible(false);
        m_filtersUnfoldBtn->setVisible(false);
        m_exposureBtn->setVisible(false);
        m_unfoldBtn->setVisible(true);
        setFixedSize(QSize(m_unfoldBtn->width(), m_unfoldBtn->height()));

        group->start(QAbstractAnimation::DeleteWhenStopped);
    });
}

void takePhotoSettingAreaWidget::showUnfold(bool bShow, circlePushButton *btn, bool isShortcut)
{
    circlePushBtnList btnList;
    btnList.push_back(m_flashlightUnfoldBtn);
    btnList.push_back(m_delayUnfoldBtn);
    btnList.push_back(m_filtersUnfoldBtn);
    btnList.push_back(m_exposureBtn);
    btnList.push_back(m_foldBtn);

    // 以哪个按钮为原点来展开动画
    circlePushButton* originBtn = m_filtersUnfoldBtn;
    if (!m_bPhoto)
        originBtn = m_delayUnfoldBtn;

    //位移动画
    QPoint startPos = QPoint(0, (originBtn->height() + m_btnHeightOffset) * 2); //开始位置固定为中间按钮
    if (!m_bPhoto)
        startPos = QPoint(0,0);

    QEasingCurve ec;
    ec.setType(QEasingCurve::OutBack);
    ec.setOvershoot(EASING_OVERSHOOT);

    QParallelAnimationGroup *pPosGroup = new QParallelAnimationGroup(this);
    int index = 0;
    for (auto btn : btnList) {
        if (!m_bPhoto) {
            if (btn == m_flashlightUnfoldBtn
                    || btn == m_filtersUnfoldBtn
                    || btn == m_exposureBtn)
                continue;
        }

        QPropertyAnimation *opacity = new QPropertyAnimation(btn, "opacity", this);
        opacity->setDuration(ANIMATION_FOLD_DURATION);
        opacity->setStartValue(0);
        opacity->setEndValue(102);
        pPosGroup->addAnimation(opacity);

        QPropertyAnimation *iconOpacity = new QPropertyAnimation(btn, "iconopacity", this);
        iconOpacity->setDuration(ANIMATION_FOLD_DURATION);
        iconOpacity->setStartValue(0);
        iconOpacity->setEndValue(1);
        btn->setIconOpacity(0);
        pPosGroup->addAnimation(iconOpacity);

        if (btn == originBtn) {
            originBtn->move(0,(originBtn->height() + m_btnHeightOffset) * index++);
            continue;
        }

        QPropertyAnimation* position = new QPropertyAnimation(btn, "pos", this);
        position->setDuration(ANIMATION_FOLD_DURATION);
        position->setStartValue(startPos);
        if (m_foldBtn == btn)
            //折叠按钮与上一按钮间距比普通间距多10px
            position->setEndValue(QPoint(0, 10 + (originBtn->height() + m_btnHeightOffset) * index++));
        else
            position->setEndValue(QPoint(0, (originBtn->height() + m_btnHeightOffset) * index++));
        position->setEasingCurve(ec);
        pPosGroup->addAnimation(position);
    }

    connect(pPosGroup, &QParallelAnimationGroup::finished, this, [=](){
        m_isBtnsFold = false;
        if (isShortcut) {
            btn->setFocus();
        }
    });

    m_foldBtn->setVisible(bShow);
    if(m_bPhoto) {
        m_flashlightUnfoldBtn->setVisible(bShow);
        m_filtersUnfoldBtn->setVisible(bShow);
        m_exposureBtn->setVisible(bShow);
    } else {
        m_flashlightUnfoldBtn->setVisible(!bShow);
        m_filtersUnfoldBtn->setVisible(!bShow);
        m_exposureBtn->setVisible(!bShow);
    }
    m_delayUnfoldBtn->setVisible(bShow);
    m_unfoldBtn->setVisible(!bShow);
    pPosGroup->start(QAbstractAnimation::DeleteWhenStopped);

    int nBtnCount = btnList.size();
    if (!m_bPhoto)
        nBtnCount -= 3;
    //折叠按钮与上一按钮间距比普通间距多10px 整体高度+10 回弹伸缩范围 +5
    setFixedSize(QSize(originBtn->width(), 10 + 5 + originBtn->height() * nBtnCount + (nBtnCount - 1) * m_btnHeightOffset));
    update();
}

void takePhotoSettingAreaWidget::showDelayButtons(bool bShow, bool isShortcut)
{
    //位移动画
    QPropertyAnimation *position1 = new QPropertyAnimation(m_noDelayBtn, "pos", this);
    QPropertyAnimation *position2 = new QPropertyAnimation(m_delay3SecondBtn, "pos", this);
    QPropertyAnimation *position3 = new QPropertyAnimation(m_delay6SecondBtn, "pos", this);
    //透明度动画
    QPropertyAnimation *opacity = new QPropertyAnimation(this, "opacity", this);

    QPropertyAnimation *opacity1 = new QPropertyAnimation(m_noDelayBtn, "iconopacity", this);
    QPropertyAnimation *opacity2 = new QPropertyAnimation(m_delay3SecondBtn, "iconopacity", this);
    QPropertyAnimation *opacity3 = new QPropertyAnimation(m_delay6SecondBtn, "iconopacity", this);
    QParallelAnimationGroup *pPosGroup = new QParallelAnimationGroup(this);

    QEasingCurve ec;
    ec.setType(QEasingCurve::OutBack);
    ec.setOvershoot(EASING_OVERSHOOT);

    if (bShow) {
        position1->setDuration(ANIMATION_DURATION);
        position1->setStartValue(QPoint(0, 0));
        position1->setEndValue(QPoint(0, m_delayFoldBtn->height() + m_threeBtnOffset));
        position1->setEasingCurve(ec);

        position2->setDuration(ANIMATION_DURATION);
        position2->setStartValue(QPoint(0, 0));
        position2->setEndValue(QPoint(0, m_delayFoldBtn->height() * 2 + m_threeBtnOffset * 2));
        position2->setEasingCurve(ec);

        position3->setDuration(ANIMATION_DURATION);
        position3->setStartValue(QPoint(0, 0));
        position3->setEndValue(QPoint(0, m_delayFoldBtn->height() * 3 + m_threeBtnOffset * 3));
        position3->setEasingCurve(ec);

        opacity->setDuration(ANIMATION_DURATION);
        opacity->setStartValue(0);
        opacity->setEndValue(102);

        opacity1->setDuration(ANIMATION_DURATION);
        opacity1->setStartValue(0);
        opacity1->setEndValue(1);

        opacity2->setDuration(ANIMATION_DURATION);
        opacity2->setStartValue(0);
        opacity2->setEndValue(1);

        opacity3->setDuration(ANIMATION_DURATION);
        opacity3->setStartValue(0);
        opacity3->setEndValue(1);

        m_delayFoldBtn->setVisible(bShow);
        m_noDelayBtn->setVisible(bShow);
        m_delay3SecondBtn->setVisible(bShow);
        m_delay6SecondBtn->setVisible(bShow);
        m_noDelayBtn->setIconOpacity(0);
        m_delay3SecondBtn->setIconOpacity(0);
        m_delay6SecondBtn->setIconOpacity(0);
        connect(pPosGroup, &QPropertyAnimation::finished, this, [=]{
            if (isShortcut)
                m_delayFoldBtn->setFocus();
        });
    } else {
        position1->setDuration(ANIMATION_DURATION);
        position1->setStartValue(QPoint(0, m_delayFoldBtn->height() + m_threeBtnOffset));
        position1->setEndValue(QPoint(0, 0));
        position1->setEasingCurve(ec);

        position2->setDuration(ANIMATION_DURATION);
        position2->setStartValue(QPoint(0, m_delayFoldBtn->height() * 2 + m_threeBtnOffset * 2));
        position2->setEndValue(QPoint(0, 0));
        position2->setEasingCurve(ec);

        position3->setDuration(ANIMATION_DURATION);
        position3->setStartValue(QPoint(0, m_delayFoldBtn->height() * 3 + m_threeBtnOffset * 3));
        position3->setEndValue(QPoint(0, 0));
        position3->setEasingCurve(ec);

        opacity->setDuration(ANIMATION_DURATION);
        opacity->setStartValue(102);
        opacity->setEndValue(0);

        opacity1->setDuration(ANIMATION_DURATION);
        opacity1->setStartValue(1);
        opacity1->setEndValue(0);

        opacity2->setDuration(ANIMATION_DURATION);
        opacity2->setStartValue(1);
        opacity2->setEndValue(0);

        opacity3->setDuration(ANIMATION_DURATION);
        opacity3->setStartValue(1);
        opacity3->setEndValue(0);

        connect(pPosGroup, &QPropertyAnimation::finished, this, [=](){
            m_delayFoldBtn->setVisible(bShow);
            m_noDelayBtn->setVisible(bShow);
            m_delay3SecondBtn->setVisible(bShow);
            m_delay6SecondBtn->setVisible(bShow);
            m_delayGroupDisplay = false;
            showUnfold(true, m_delayUnfoldBtn, isShortcut);
        });
    }

    //normal状态下设置按钮透明，只显示背景颜色
    m_delayFoldBtn->setOpacity(255 * 0.4);
    m_noDelayBtn->setbackground(Qt::transparent);
    m_delay3SecondBtn->setbackground(Qt::transparent);
    m_delay6SecondBtn->setbackground(Qt::transparent);

    pPosGroup->addAnimation(position1);
    pPosGroup->addAnimation(position2);
    pPosGroup->addAnimation(position3);
    pPosGroup->addAnimation(opacity);
    pPosGroup->addAnimation(opacity1);
    pPosGroup->addAnimation(opacity2);
    pPosGroup->addAnimation(opacity3);

    pPosGroup->start(QAbstractAnimation::DeleteWhenStopped);

    setFixedSize(QSize(m_delayFoldBtn->width(), m_delayFoldBtn->height() * 4 + m_threeBtnOffset * 3 + 2));
    update();
}

void takePhotoSettingAreaWidget::showFlashlights(bool bShow, bool isShortcut)
{
    QPropertyAnimation *position1 = new QPropertyAnimation(m_flashlightOnBtn, "pos", this);
    QPropertyAnimation *position2 = new QPropertyAnimation(m_flashlightOffBtn, "pos", this);

    QPropertyAnimation *opacity = new QPropertyAnimation(this, "opacity", this);

    QPropertyAnimation *opacity1 = new QPropertyAnimation(m_flashlightOnBtn, "iconopacity", this);
    QPropertyAnimation *opacity2 = new QPropertyAnimation(m_flashlightOffBtn, "iconopacity", this);
    QParallelAnimationGroup *pPosGroup = new QParallelAnimationGroup(this);

    QEasingCurve ec;
    ec.setType(QEasingCurve::OutBack);
    ec.setOvershoot(EASING_OVERSHOOT);

    if (bShow) {
        position1->setDuration(ANIMATION_DURATION);
        position1->setStartValue(QPoint(0, 0));
        position1->setEndValue(QPoint(0, m_delayFoldBtn->height() + m_threeBtnOffset));
        position1->setEasingCurve(ec);

        position2->setDuration(ANIMATION_DURATION);
        position2->setStartValue(QPoint(0, 0));
        position2->setEndValue(QPoint(0, m_delayFoldBtn->height() * 2 + m_threeBtnOffset * 2));
        position2->setEasingCurve(ec);

        opacity->setDuration(ANIMATION_DURATION);
        opacity->setStartValue(0);
        opacity->setEndValue(102);

        opacity1->setDuration(ANIMATION_DURATION);
        opacity1->setStartValue(0);
        opacity1->setEndValue(1);

        opacity2->setDuration(ANIMATION_DURATION);
        opacity2->setStartValue(0);
        opacity2->setEndValue(1);

        m_flashlightFoldBtn->setVisible(bShow);
        m_flashlightOnBtn->setVisible(bShow);
        m_flashlightOffBtn->setVisible(bShow);
        m_flashlightOnBtn->setIconOpacity(0);
        m_flashlightOffBtn->setIconOpacity(0);

        connect(pPosGroup, &QPropertyAnimation::finished, this, [=]{
            if (isShortcut)
                m_flashlightFoldBtn->setFocus();
        });
    } else {
        position1->setDuration(ANIMATION_DURATION);
        position1->setStartValue(QPoint(0, m_delayFoldBtn->height() + m_threeBtnOffset));
        position1->setEndValue(QPoint(0, 0));
        position1->setEasingCurve(ec);

        position2->setDuration(ANIMATION_DURATION);
        position2->setStartValue(QPoint(0, m_delayFoldBtn->height() * 2 + m_threeBtnOffset * 2));
        position2->setEndValue(QPoint(0, 0));
        position2->setEasingCurve(ec);

        opacity->setDuration(ANIMATION_DURATION);
        opacity->setStartValue(102);
        opacity->setEndValue(0);

        opacity1->setDuration(ANIMATION_DURATION);
        opacity1->setStartValue(1);
        opacity1->setEndValue(0);

        opacity2->setDuration(ANIMATION_DURATION);
        opacity2->setStartValue(1);
        opacity2->setEndValue(0);

        connect(pPosGroup, &QPropertyAnimation::finished, this, [=]{
            m_flashlightFoldBtn->setVisible(bShow);
            m_flashlightOnBtn->setVisible(bShow);
            m_flashlightOffBtn->setVisible(bShow);
            m_flashGroupDisplay = false;
            showUnfold(true, m_flashlightUnfoldBtn, isShortcut);
        });
    }

    pPosGroup->addAnimation(position1);
    pPosGroup->addAnimation(position2);
    pPosGroup->addAnimation(opacity);
    pPosGroup->addAnimation(opacity1);
    pPosGroup->addAnimation(opacity2);

    pPosGroup->start(QAbstractAnimation::DeleteWhenStopped);

    setFixedSize(QSize(m_flashlightFoldBtn->width(), m_flashlightFoldBtn->height() * 3 + m_threeBtnOffset * 2 + 2));
    update();
}

void takePhotoSettingAreaWidget::showFilters(bool bShow, bool isShortcut)
{
    QPropertyAnimation *posClose = new QPropertyAnimation(m_filtersCloseBtn, "pos", this);
    QPropertyAnimation *opacityClose = new QPropertyAnimation(m_filtersCloseBtn, "opacity", this);
    QPropertyAnimation *iconOpacityClose = new QPropertyAnimation(m_filtersCloseBtn, "iconopacity", this);

    QPropertyAnimation *posScroll = new QPropertyAnimation(m_scrollArea, "geometry", this);
    QPropertyAnimation *opacityScroll = new QPropertyAnimation(m_scrollArea, "opacity", this);
    QPropertyAnimation *iconOpacityScroll = new QPropertyAnimation(m_scrollArea, "iconopacity", this);

    //透明度动画
    QPropertyAnimation *posFold = new QPropertyAnimation(m_filtersFoldBtn, "pos", this);
    posFold->setDuration(ANIMATION_DURATION);
    posFold->setStartValue(QPoint(5, 0));
    posFold->setEndValue(QPoint(5, 0));
    QPropertyAnimation *opacityFold = new QPropertyAnimation(m_filtersFoldBtn, "opacity", this);
    QPropertyAnimation *iconOpacityFold = new QPropertyAnimation(m_filtersFoldBtn, "iconopacity", this);
    QParallelAnimationGroup *pPosGroup = new QParallelAnimationGroup(this);

    QEasingCurve ec;
    ec.setType(QEasingCurve::OutBack);
    ec.setOvershoot(EASING_OVERSHOOT);

    //Rolling window start/end position
    QRect startGeometry, endGeometry;
    QPoint startPoint = m_filtersFoldBtn->rect().bottomLeft();
    startPoint.setY(startPoint.y() + m_threeBtnOffset);
    startGeometry.setTopLeft(startPoint);
    startGeometry.setHeight(0);
    startGeometry.setWidth(m_filterPreviewBtnList.first()->width());

    endGeometry.setTopLeft(startGeometry.topLeft());
    endGeometry.setHeight(m_scrollHeight);
    endGeometry.setWidth(startGeometry.width());

    if (bShow) {
        posScroll->setDuration(ANIMATION_FILTER_DURATION);
        posScroll->setEasingCurve(ec);
        posScroll->setStartValue(startGeometry);
        posScroll->setEndValue(endGeometry);

        opacityScroll->setDuration(ANIMATION_FILTER_DURATION);
        opacityScroll->setStartValue(0);
        opacityScroll->setEndValue(102);

        iconOpacityScroll->setDuration(ANIMATION_FILTER_DURATION);
        iconOpacityScroll->setStartValue(0);
        iconOpacityScroll->setEndValue(1);

        posClose->setDuration(ANIMATION_FILTER_DURATION);
        posClose->setEasingCurve(ec);
        posClose->setStartValue(QPoint(5, 0));
        posClose->setEndValue(QPoint(5, (m_filtersFoldBtn->height() + m_threeBtnOffset*2) + endGeometry.height() + 10));

        opacityFold->setDuration(ANIMATION_FILTER_DURATION);
        opacityFold->setStartValue(0);
        opacityFold->setEndValue(102);

        iconOpacityFold->setDuration(ANIMATION_FILTER_DURATION);
        iconOpacityFold->setStartValue(0);
        iconOpacityFold->setEndValue(1);

        opacityClose->setDuration(ANIMATION_FILTER_DURATION);
        opacityClose->setStartValue(0);
        opacityClose->setEndValue(102);

        iconOpacityClose->setDuration(ANIMATION_FILTER_DURATION);
        iconOpacityClose->setStartValue(0);
        iconOpacityClose->setEndValue(1);

        m_filtersFoldBtn->setVisible(bShow);
        m_filtersFoldBtn->setOpacity(0);
        m_filtersFoldBtn->setIconOpacity(0);
        for (auto pBtn : m_filterPreviewBtnList) {
            pBtn->setVisible(bShow);
        }
        m_filtersCloseBtn->setVisible(bShow);
        m_filtersCloseBtn->setOpacity(0);
        m_filtersCloseBtn->setIconOpacity(0);
        m_scrollArea->setVisible(bShow);
        connect(pPosGroup, &QPropertyAnimation::finished, this, [=]{
            for (auto pBtn : m_filterPreviewBtnList) {
                pBtn->update();
                pBtn->clearFocus();
            }
        });

        if (isShortcut) {
            QTimer::singleShot(ANIMATION_FILTER_DURATION, this, [=] {
                m_filtersFoldBtn->setFocus();
            });
        }
    } else {
        posClose->setDuration(ANIMATION_FILTER_DURATION);
        posClose->setEasingCurve(ec);
        posClose->setStartValue(QPoint(5, (m_filtersFoldBtn->height() + m_threeBtnOffset*2) + endGeometry.height()));
        posClose->setEndValue(QPoint(5, 0));

        posScroll->setDuration(ANIMATION_FILTER_DURATION);
        posScroll->setStartValue(endGeometry);
        posScroll->setEndValue(startGeometry);

        opacityScroll->setDuration(ANIMATION_FILTER_DURATION);
        opacityScroll->setStartValue(102);
        opacityScroll->setEndValue(0);

        iconOpacityScroll->setDuration(ANIMATION_FILTER_DURATION);
        iconOpacityScroll->setStartValue(1);
        iconOpacityScroll->setEndValue(0);

        opacityFold->setDuration(ANIMATION_FILTER_DURATION);
        opacityFold->setStartValue(102);
        opacityFold->setEndValue(0);

        iconOpacityFold->setDuration(ANIMATION_FILTER_DURATION);
        iconOpacityFold->setStartValue(1);
        iconOpacityFold->setEndValue(0);

        opacityClose->setDuration(ANIMATION_FILTER_DURATION);
        opacityClose->setStartValue(102);
        opacityClose->setEndValue(0);

        iconOpacityClose->setDuration(ANIMATION_FILTER_DURATION);
        iconOpacityClose->setStartValue(1);
        iconOpacityClose->setEndValue(0);

        m_filtersCloseBtn->setVisible(bShow);
        connect(pPosGroup, &QPropertyAnimation::finished, this, [=](){
            m_filtersFoldBtn->setVisible(bShow);
            for (auto pBtn : m_filterPreviewBtnList)
                pBtn->setVisible(bShow);
            m_filtersCloseBtn->setVisible(bShow);
            m_scrollArea->setVisible(bShow);
            m_filtersGroupDislay = false;
            showUnfold(true, m_filtersUnfoldBtn, isShortcut);
        });
    }

    pPosGroup->addAnimation(posFold);
    pPosGroup->addAnimation(opacityFold);
    pPosGroup->addAnimation(iconOpacityFold);

    pPosGroup->addAnimation(posScroll);
    pPosGroup->addAnimation(opacityScroll);
    pPosGroup->addAnimation(iconOpacityScroll);

    pPosGroup->addAnimation(posClose);
    pPosGroup->addAnimation(opacityClose);
    pPosGroup->addAnimation(iconOpacityClose);

    pPosGroup->start(QAbstractAnimation::DeleteWhenStopped);

    emit sngShowFilterName(bShow);

    // 每个按钮增加回弹间隙1像素
    int height = (m_filtersFoldBtn->height() + m_threeBtnOffset*2 + 1) * 2 + endGeometry.height();
    setFixedSize(QSize(m_filterPreviewBtnList.last()->width(), height));
    update();
}

void takePhotoSettingAreaWidget::foldBtnClicked(bool isShortcut)
{
    //closeAllGroup();
    showFold(true, isShortcut);
}

void takePhotoSettingAreaWidget::unfoldBtnClicked(bool isShortcut)
{
    QPropertyAnimation * opacity = new QPropertyAnimation(m_unfoldBtn, "opacity", this);
    opacity->setStartValue(102);
    opacity->setEndValue(0);
    opacity->setDuration(ANIMATION_ROTATE_DURATION);

    QPropertyAnimation *rotate = new QPropertyAnimation(m_unfoldBtn, "rotate", this);
    rotate->setStartValue(0);
    rotate->setEndValue(90);
    rotate->setDuration(ANIMATION_ROTATE_DURATION);

    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
    group->addAnimation(opacity);
    group->addAnimation(rotate);

    group->start(QAbstractAnimation::DeleteWhenStopped);

    connect(group, &QParallelAnimationGroup::finished, this, [=](){
        if (m_bPhoto) {
            showUnfold(true, m_flashlightUnfoldBtn, isShortcut);
        } else {
            showUnfold(true, m_delayUnfoldBtn, isShortcut);
        }
        group->deleteLater();

    });
}

void takePhotoSettingAreaWidget::flashlightFoldBtnClicked(bool isShortcut)
{
    showFlashlights(false, isShortcut);
}

void takePhotoSettingAreaWidget::flashlightUnfoldBtnClicked(bool isShortcut)
{
    hideAll();
    closeAllGroup();
    showFlashlights(true, isShortcut);
    m_flashGroupDisplay = true;

    //normal状态下设置按钮透明，只显示背景颜色
    m_flashlightFoldBtn->setOpacity(255 * 0.4);
    m_flashlightOnBtn->setbackground(Qt::transparent);
    m_flashlightOffBtn->setbackground(Qt::transparent);
}

void takePhotoSettingAreaWidget::delayUnfoldBtnClicked(bool isShortcut)
{
    hideAll();
    closeAllGroup();
    showDelayButtons(true, isShortcut);
    m_delayGroupDisplay = true;
}

void takePhotoSettingAreaWidget::delayfoldBtnClicked(bool isShortcut)
{
    showDelayButtons(false, isShortcut);
}

void takePhotoSettingAreaWidget::filtersUnfoldBtnClicked(bool isShortcut)
{
    hideAll();
    closeAllGroup();
    m_filtersGroupDislay = true;
    showFilters(true, isShortcut);
    emit sigFilterGroupDisplay(true);
}

void takePhotoSettingAreaWidget::filtersFoldBtnClicked(bool isShortcut)
{
    showFilters(false, isShortcut);
    emit sigFilterGroupDisplay(false);
}

void takePhotoSettingAreaWidget::hideAll()
{
    m_foldBtn->setVisible(false);
    m_unfoldBtn->setVisible(false);

    m_flashlightFoldBtn->setVisible(false);
    m_flashlightUnfoldBtn->setVisible(false);
    m_flashlightOnBtn->setVisible(false);
    m_flashlightOffBtn->setVisible(false);

    m_delayFoldBtn->setVisible(false);
    m_delayUnfoldBtn->setVisible(false);
    m_noDelayBtn->setVisible(false);
    m_delay3SecondBtn->setVisible(false);
    m_delay6SecondBtn->setVisible(false);

    m_filtersFoldBtn->setVisible(false);
    m_filtersUnfoldBtn->setVisible(false);
    for (auto btn : m_filterPreviewBtnList)
        btn->setVisible(false);
    m_filtersCloseBtn->setVisible(false);

    m_exposureBtn->setVisible(false);
}

void takePhotoSettingAreaWidget::keyDownClick()
{
    if (!m_isBtnsFold) {
        if (m_flashlightUnfoldBtn->hasFocus()) {
            m_delayUnfoldBtn->setFocus();
        } else if(m_delayUnfoldBtn->hasFocus()){
            m_filtersUnfoldBtn->setFocus();
        } else if (m_filtersUnfoldBtn->hasFocus()) {
            m_exposureBtn->setFocus();
        } else if (m_exposureBtn->hasFocus()) {
            m_foldBtn->setFocus();
        } else if(m_foldBtn->hasFocus()){
            m_flashlightUnfoldBtn->setFocus();
        }
    }

    if (m_flashGroupDisplay) {
        if (m_flashlightFoldBtn->hasFocus()) {
            m_flashlightOnBtn->setFocus();
        } else if(m_flashlightOnBtn->hasFocus()){
            m_flashlightOffBtn->setFocus();
        } else if(m_flashlightOffBtn->hasFocus()){
            m_flashlightFoldBtn->setFocus();
        }
    }

    if (m_delayGroupDisplay) {
        if (m_delayFoldBtn->hasFocus()) {
            m_noDelayBtn->setFocus();
        } else if(m_noDelayBtn->hasFocus()) {
            m_delay3SecondBtn->setFocus();
        } else if(m_delay3SecondBtn->hasFocus()) {
            m_delay6SecondBtn->setFocus();
        } else if(m_delay6SecondBtn->hasFocus()) {
            m_delayFoldBtn->setFocus();
        }
    }

    if (m_filtersGroupDislay) {
        if (m_filtersFoldBtn->hasFocus()) {
            m_filterPreviewBtnList.at(0)->setFocus();
            m_scrollArea->verticalScrollBar()->setValue(0);
        } else {
            for (int i = 0; i < efilterType::filter_Count; i++) {
                if (m_filterPreviewBtnList.at(i)->hasFocus()) {
                    if (i + 1 < efilterType::filter_Count) {
                        m_filterPreviewBtnList.at(i + 1)->setFocus();
                        int value = (m_filtersFoldBtn->height() + m_threeBtnOffset * 2) * (i + 2) + 15 -
                                m_scrollArea->height() + m_scrollArea->verticalScrollBar()->value();
                        if (value) {
                            m_scrollArea->verticalScrollBar()->setValue(m_scrollArea->verticalScrollBar()->value() + value);
                        }
                        return;
                    } else {
                        m_filtersCloseBtn->setFocus();
                        return;
                    }
                }
            }

            if (m_filtersCloseBtn->hasFocus())
                m_filtersFoldBtn->setFocus();
        }
    }
}

void takePhotoSettingAreaWidget::keyUpClick()
{
    if (!m_isBtnsFold) {
        if (m_flashlightUnfoldBtn->hasFocus()) {
            m_foldBtn->setFocus();
        } else if(m_exposureBtn->hasFocus()){
            m_filtersUnfoldBtn->setFocus();
        } else if(m_filtersUnfoldBtn->hasFocus()){
            m_delayUnfoldBtn->setFocus();
        } else if(m_delayUnfoldBtn->hasFocus()){
            m_flashlightUnfoldBtn->setFocus();
        } else if (m_foldBtn->hasFocus()) {
            m_exposureBtn->setFocus();
        }
    }

    if (m_flashGroupDisplay) {
        if (m_flashlightFoldBtn->hasFocus()) {
            m_flashlightOffBtn->setFocus();
        } else if(m_flashlightOnBtn->hasFocus()){
            m_flashlightFoldBtn->setFocus();
        } else if(m_flashlightOffBtn->hasFocus()){
            m_flashlightOnBtn->setFocus();
        }
    }

    if (m_delayGroupDisplay) {
        if (m_delayFoldBtn->hasFocus()) {
            m_delay6SecondBtn->setFocus();
        } else if(m_noDelayBtn->hasFocus()) {
            m_delayFoldBtn->setFocus();
        } else if(m_delay3SecondBtn->hasFocus()) {
            m_noDelayBtn->setFocus();
        } else if(m_delay6SecondBtn->hasFocus()) {
            m_delay3SecondBtn->setFocus();
        }
    }

    if (m_filtersGroupDislay) {
        if (m_filtersFoldBtn->hasFocus()) {
            m_filtersCloseBtn->setFocus();
        } else {
            for (int i = efilterType::filter_Count - 1; i >= 0; i--) {
                if (m_filterPreviewBtnList.at(i)->hasFocus()) {
                    if (i - 1 >= 0) {
                        m_filterPreviewBtnList.at(i - 1)->setFocus();
                        int value = m_scrollArea->verticalScrollBar()->value() - (m_filtersFoldBtn->height() + m_threeBtnOffset * 2) * (i - 1);
                        if (value) {
                            m_scrollArea->verticalScrollBar()->setValue(m_scrollArea->verticalScrollBar()->value() - value);
                        }
                        return;
                    } else {
                        m_filtersFoldBtn->setFocus();
                        return;
                    }
                }
            }

            if (m_filtersCloseBtn->hasFocus()) {
                m_filterPreviewBtnList.at(efilterType::filter_Count - 1)->setFocus();
                m_scrollArea->verticalScrollBar()->setValue(m_scrollArea->verticalScrollBar()->maximum());
            }
        }
    }
}

void takePhotoSettingAreaWidget::keyEnterClick()
{
    if (m_unfoldBtn->hasFocus()) {
        unfoldBtnClicked(true);
    } else if (m_foldBtn->hasFocus()) {
        foldBtnClicked(true);
    } else if (m_flashlightUnfoldBtn->hasFocus()) {
        flashlightUnfoldBtnClicked(true);
    } else if (m_flashlightFoldBtn->hasFocus()) {
        flashlightFoldBtnClicked(true);
    } else if (m_flashlightOnBtn->hasFocus()) {
        emit m_flashlightOnBtn->clicked(true);
    } else if (m_flashlightOffBtn->hasFocus()) {
        emit m_flashlightOffBtn->clicked(true);
    } else if (m_delayFoldBtn->hasFocus()) {
        delayfoldBtnClicked(true);
    } else if (m_delayUnfoldBtn->hasFocus()) {
        delayUnfoldBtnClicked(true);
    } else if (m_noDelayBtn->hasFocus()) {
        emit m_noDelayBtn->clicked(true);
    } else if (m_delay3SecondBtn->hasFocus()) {
        emit m_delay3SecondBtn->clicked(true);
    } else if (m_delay6SecondBtn->hasFocus()) {
        emit m_delay6SecondBtn->clicked(true);
    } else if (m_filtersFoldBtn->hasFocus()) {
        filtersFoldBtnClicked(true);
    } else if (m_filtersUnfoldBtn->hasFocus()) {
        filtersUnfoldBtnClicked(true);
    } else if (filterPreviewButton* pFocusBtn = getFilterPreviewFocusBtn()) {
        emit pFocusBtn->clicked(true);
    } else if (m_filtersCloseBtn->hasFocus()) {
        filtersFoldBtnClicked(true);
    } else if (m_exposureBtn->hasFocus()) {
        exposureBtnClicked(true);
    } else if (/*曝光滑块有焦点*/false) {
        // TODO: 执行设置曝光值函数
    }
}

void takePhotoSettingAreaWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::HighQualityAntialiasing, true);

    int width, height;

    if (m_delayGroupDisplay) {
        QPainterPath path;  //上半圆+矩形
        QPainterPath arc1;  //下半圆

        width = m_delayFoldBtn->width();
        height = m_delayFoldBtn->height();

        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(m_buttonGroupColor));

        path.moveTo(m_delayFoldBtn->rect().center());
        path.arcTo(m_delayFoldBtn->rect(), 0, 180);
        path.lineTo(m_delay6SecondBtn->x(), m_delay6SecondBtn->y() + height / 2);
        path.lineTo(m_delay6SecondBtn->x() + width, m_delay6SecondBtn->y() + height / 2);
        path.lineTo(m_delayFoldBtn->x() + width, m_delayFoldBtn->y() + height / 2);

        arc1.moveTo(m_delay6SecondBtn->x() + width / 2, m_delay6SecondBtn->y() + height / 2);
        arc1.arcTo(QRect(m_delay6SecondBtn->x(), m_delay6SecondBtn->y(), width, height), 180, 180);

        path.addPath(arc1);
        painter.fillPath(path, QBrush(m_buttonGroupColor));

    } else if (m_flashGroupDisplay) {
        QPainterPath path;
        QPainterPath arc1;

        width = m_flashlightFoldBtn->width();
        height = m_flashlightFoldBtn->height();

        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(m_buttonGroupColor));

        path.moveTo(m_flashlightFoldBtn->rect().center());
        path.arcTo(m_flashlightFoldBtn->rect(), 0, 180);
        path.lineTo(m_flashlightOffBtn->x(), m_flashlightOffBtn->y() + height / 2);
        path.lineTo(m_flashlightOffBtn->x() + width, m_flashlightOffBtn->y() + height / 2);
        path.lineTo(m_flashlightFoldBtn->x() + width, m_flashlightFoldBtn->y() + height / 2);

        arc1.moveTo(m_flashlightOffBtn->x() + width / 2, m_flashlightOffBtn->y() + height / 2);
        arc1.arcTo(QRect(m_flashlightOffBtn->x(), m_flashlightOffBtn->y(), width, height), 180, 180);

        path.addPath(arc1);
        painter.fillPath(path, QBrush(m_buttonGroupColor));
    } else if (m_filtersGroupDislay) {

    }
}

void takePhotoSettingAreaWidget::onDelayBtnsClicked(bool isShortcut)
{
    circlePushButton *pBtn = static_cast<circlePushButton *>(sender());
    if (nullptr == pBtn)
        return;
    pBtn == m_noDelayBtn ? m_noDelayBtn->setSelected(true) : m_noDelayBtn->setSelected(false);
    pBtn == m_delay3SecondBtn ? m_delay3SecondBtn->setSelected(true) : m_delay3SecondBtn->setSelected(false);
    pBtn == m_delay6SecondBtn ? m_delay6SecondBtn->setSelected(true) : m_delay6SecondBtn->setSelected(false);

    int delayTime = 0;
    circlePushButton *bCopyBtn = m_noDelayBtn;

    if (m_delay3SecondBtn->getButtonState()) {
        delayTime = 3;
        bCopyBtn = m_delay3SecondBtn;
    }

    if (m_delay6SecondBtn->getButtonState()) {
        delayTime = 6;
        bCopyBtn = m_delay6SecondBtn;
    }

    m_delayUnfoldBtn->copyPixmap(*bCopyBtn);
    m_delayFoldBtn->copyPixmap(*bCopyBtn);
    emit sngSetDelayTakePhoto(delayTime);
    update();

    showDelayButtons(false, isShortcut); //三级菜单选中后跳转到上一级菜单
}

void takePhotoSettingAreaWidget::onFlashlightBtnsClicked(bool isShortcut)
{
    auto pBtn = static_cast<circlePushButton *>(sender());
    if (nullptr == pBtn)
        return;

    pBtn == m_flashlightOnBtn ? m_flashlightOnBtn->setSelected(true) : m_flashlightOnBtn->setSelected(false);
    pBtn == m_flashlightOffBtn ? m_flashlightOffBtn->setSelected(true) : m_flashlightOffBtn->setSelected(false);

    setFlashlight(m_flashlightOnBtn->getButtonState());
    emit sngSetFlashlight(m_flashlightOnBtn->getButtonState());

    showFlashlights(false, isShortcut);
}

void takePhotoSettingAreaWidget::onFilterBtnsClicked(bool isShortcut)
{
    Q_UNUSED(isShortcut);

    auto pBtn = static_cast<filterPreviewButton *>(sender());
    if (!pBtn)
        return;

    for (auto pTmpBtn : m_filterPreviewBtnList) {
        pTmpBtn->setSelected(pTmpBtn == pBtn);
    }

    efilterType newFilterType = pBtn->getFiltertype();
    if (m_filterType != newFilterType) {
        emit sngFilterChanged(newFilterType);
        emit sngSetFilterName(filterPreviewButton::filterName(newFilterType));
        m_filterType = newFilterType;
    }
}

void takePhotoSettingAreaWidget::onUpdateFilterImage(QImage *img)
{
    if (!img)
        return;

    for (auto btn : m_filterPreviewBtnList) {
        QImage tmp = img->copy();
        btn->setImage(&tmp);
    }
}

void takePhotoSettingAreaWidget::onExposureValueChanged(int value)
{
    if (value)
        m_exposureBtn->setNewNotification(true);
    else
        m_exposureBtn->setNewNotification(false);
}

filterPreviewButton *takePhotoSettingAreaWidget::getFilterPreviewFocusBtn()
{
    for (auto pBtn : m_filterPreviewBtnList) {
        if (pBtn->hasFocus())
            return pBtn;
    }

    return nullptr;
}

void takePhotoSettingAreaWidget::exposureBtnClicked(bool isShortcut)
{
    if (QDateTime::currentMSecsSinceEpoch() - m_animationTime < 500) //防止上一次动画未完成
        return;
    else
        m_animationTime = QDateTime::currentMSecsSinceEpoch();

    QPoint pos = mapToParent(m_exposureBtn->pos());
    m_exposureSlider->move(pos.x() + m_exposureBtn->width() + 8, pos.y() - (EXPOSURE_SLIDER_HEIGHT - m_exposureBtn->height()) / 2);

    QRect start = QRect(QPoint(pos.x() + m_exposureBtn->width() + 8, pos.y() - (EXPOSURE_SLIDER_HEIGHT - m_exposureBtn->height()) / 2), QSize(m_exposureSlider->width(), 0));
    QRect end = start;
    end.setHeight(EXPOSURE_SLIDER_HEIGHT);

    if (m_exposureSliderDisplay) {
        m_exposureSlider->showContent(false, isShortcut);

        connect(m_exposureSlider, &ExposureSlider::contentHided, this, [=](){
            QPropertyAnimation *opacity = new QPropertyAnimation(m_exposureSlider, "opacity", this);
            opacity->setStartValue(102);
            opacity->setEndValue(0);
            opacity->setDuration(SLIDER_ANIMATION_DURATION);
            opacity->setEasingCurve(QEasingCurve::OutQuart);

            QPropertyAnimation *geometry = new QPropertyAnimation(m_exposureSlider, "geometry", this);
            geometry->setStartValue(end);
            geometry->setEndValue(start);
            geometry->setDuration(SLIDER_ANIMATION_DURATION);
            geometry->setEasingCurve(QEasingCurve::OutQuart);

            QParallelAnimationGroup *pGroup = new QParallelAnimationGroup(this);
            pGroup->addAnimation(opacity);
            pGroup->addAnimation(geometry);
            connect(pGroup, &QParallelAnimationGroup::finished, this, [=](){
                m_exposureSlider->hide();
                if (isShortcut) //键盘触发，焦点设回到曝光按钮上
                    m_exposureBtn->setFocus();
                pGroup->deleteLater();

                if (m_bPhotoToVideState) {
                    showUnfold(true, m_exposureBtn, isShortcut);

                    if (m_bPhoto && !m_isBtnsFold) {
                        m_flashlightUnfoldBtn->setVisible(true);
                        m_filtersUnfoldBtn->setVisible(true);
                        m_exposureBtn->setVisible(true);

                        m_flashlightUnfoldBtn->setOpacity(102);
                        m_filtersUnfoldBtn->setOpacity(102);
                        m_exposureBtn->setOpacity(102);
                    } else {
                        m_flashlightUnfoldBtn->setVisible(false);
                        m_filtersUnfoldBtn->setVisible(false);
                        m_exposureBtn->setVisible(false);
                    }

                    m_bPhotoToVideState = false;
                }
            });
            pGroup->start(QAbstractAnimation::DeleteWhenStopped);
        });

    } else {
        QPropertyAnimation *opacity = new QPropertyAnimation(m_exposureSlider, "opacity", this);
        opacity->setStartValue(0);
        opacity->setEndValue(102);
        opacity->setDuration(SLIDER_ANIMATION_DURATION);
        opacity->setEasingCurve(QEasingCurve::OutQuart);

        QPropertyAnimation *geometry = new QPropertyAnimation(m_exposureSlider, "geometry", this);
        geometry->setStartValue(start);
        geometry->setEndValue(end);
        geometry->setDuration(SLIDER_ANIMATION_DURATION);
        geometry->setEasingCurve(QEasingCurve::OutQuart);

        QParallelAnimationGroup *pGroup = new QParallelAnimationGroup(this);
        pGroup->addAnimation(opacity);
        pGroup->addAnimation(geometry);
        connect(pGroup, &QParallelAnimationGroup::finished, this, [=](){
            m_exposureSlider->showContent(true, isShortcut);
            pGroup->deleteLater();
        });

        m_exposureSlider->show();
        pGroup->start(QAbstractAnimation::DeleteWhenStopped);
    }
    m_exposureSliderDisplay = !m_exposureSliderDisplay;

    m_exposureBtn->setSelected(m_exposureSliderDisplay);
}

void takePhotoSettingAreaWidget::setDelayTime(int delayTime)
{
    m_noDelayBtn->setSelected(false);
    m_delay3SecondBtn->setSelected(false);
    m_delay6SecondBtn->setSelected(false);
    switch (delayTime) {
    case 0:
//        emit m_noDelayBtn->clicked(); //不使用触发按钮点击方式实现，防止影响动画效果
        m_noDelayBtn->setSelected(true);
        m_delayUnfoldBtn->copyPixmap(*m_noDelayBtn);
        m_delayFoldBtn->copyPixmap(*m_noDelayBtn);
        update();
        break;
    case 3:
//        emit m_delay3SecondBtn->clicked();
        m_delay3SecondBtn->setSelected(true);
        m_delayUnfoldBtn->copyPixmap(*m_delay3SecondBtn);
        m_delayFoldBtn->copyPixmap(*m_delay3SecondBtn);
        update();
        break;
    case 6:
//        emit m_delay6SecondBtn->clicked();
        m_delay6SecondBtn->setSelected(true);
        m_delayUnfoldBtn->copyPixmap(*m_delay6SecondBtn);
        m_delayFoldBtn->copyPixmap(*m_delay6SecondBtn);
        update();
        break;
    default:
        ;
    }
}

void takePhotoSettingAreaWidget::setFlashlight(bool bFlashOn)
{
    m_flashlightOnBtn->setSelected(bFlashOn);
    m_flashlightOffBtn->setSelected(!bFlashOn);
    auto p = bFlashOn ? m_flashlightOnBtn : m_flashlightOffBtn;
    m_flashlightUnfoldBtn->copyPixmap(*p);
    m_flashlightFoldBtn->copyPixmap(*p);
    update();
}

bool takePhotoSettingAreaWidget::flashLight()
{
    return m_flashlightOnBtn->getButtonState();
}

void takePhotoSettingAreaWidget::setFilterType(efilterType type)
{
    for (auto pBtn : m_filterPreviewBtnList) {
        pBtn->setSelected(pBtn->getFiltertype() == type);
    }
    update();

    m_filterType = type;
}

efilterType takePhotoSettingAreaWidget::getFilterType()
{
    return m_filterType;
}

void takePhotoSettingAreaWidget::resizeEvent(QResizeEvent *event)
{
    moveToParentLeft();
    return QWidget::resizeEvent(event);
}

void takePhotoSettingAreaWidget::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Down:
        keyDownClick();
        break;
    case Qt::Key_Up:
        keyUpClick();
        break;
    case Qt::Key_Return:
        keyEnterClick();
        break;
    }
    QWidget::keyReleaseEvent(event);
}

void takePhotoSettingAreaWidget::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event);
    if (m_isBtnsFold) {
        m_unfoldBtn->setFocus();
    } else if (!m_bPhoto) {
        if (m_delayGroupDisplay) {
            m_delayFoldBtn->setFocus();
        } else {
            m_delayUnfoldBtn->setFocus();
        }
    } else {
        if (m_flashGroupDisplay) {
            m_flashlightFoldBtn->setFocus();
        } else if (m_delayGroupDisplay) {
            m_delayFoldBtn->setFocus();
        } else if (m_filtersGroupDislay) {
            m_filtersFoldBtn->setFocus();
        }/* else if (m_exposureGroupDisplay) {
            m_exposureFoldBtn->setFocus();
        }*/ else {
            m_flashlightUnfoldBtn->setFocus();
        }
    }
    update();
}

void takePhotoSettingAreaWidget::moveToParentLeft()
{
    if (nullptr == parent())
        return;

    auto pParentWidget = static_cast<QWidget *>(parent());

    if (nullptr == pParentWidget)
        return;

    int posY = pParentWidget->height() / 2 - height() / 2;
    if (m_filtersGroupDislay)
        posY += 20;
    move(LEFT_MARGIN_PIX, posY);
}

void takePhotoSettingAreaWidget::closeAllGroup()
{
    //判断延迟拍照组是否显示
    if (m_delayGroupDisplay)
        delayfoldBtnClicked();

    //判断闪光灯拍照组是否显示
    if (m_flashGroupDisplay)
        flashlightFoldBtnClicked();

    //判断滤镜按钮组是否显示
    if (m_filtersGroupDislay)
        filtersFoldBtnClicked();

    //判断曝光按钮组是否显示
    if (m_exposureSliderDisplay)
        exposureBtnClicked();
}

void takePhotoSettingAreaWidget::setOpacity(int opacity)
{
    m_opacity = opacity;
    m_buttonGroupColor.setAlpha(opacity);
    update();
};

void takePhotoSettingAreaWidget::setState(bool bPhoto)
{
    if (m_exposureSliderDisplay) {
        //判断当前是否由拍照状态切换到录像状态
        m_bPhotoToVideState = m_bPhoto && !bPhoto;
        exposureBtnClicked();
        m_bPhoto = bPhoto;
        return;
    }

    // 若有功能按钮组处于展开状态，折叠之
    if (m_delayGroupDisplay || m_flashGroupDisplay || m_filtersGroupDislay || m_exposureSliderDisplay)
        closeAllGroup();
    // 按钮总开关处于展开状态
    else if (!m_isBtnsFold){
        // 功能按钮组未处于展开状态，重新设置各个按钮位置
        circlePushBtnList btnList;
        btnList.push_back(m_flashlightUnfoldBtn);
        btnList.push_back(m_delayUnfoldBtn);
        btnList.push_back(m_filtersUnfoldBtn);
        btnList.push_back(m_exposureBtn);
        btnList.push_back(m_foldBtn);

        int index = 0;
        for (auto btn : btnList) {
            if (!bPhoto) {
                if (btn == m_flashlightUnfoldBtn
                        || btn == m_filtersUnfoldBtn
                        || btn == m_exposureBtn)
                    continue;
            }

            btn->move(0, (btn->height() + m_btnHeightOffset) * index++);
        }

        int nBtnCount = btnList.size();
        if (!bPhoto)
            nBtnCount -= 3;
        setFixedSize(QSize(m_foldBtn->width(), m_foldBtn->height() * nBtnCount + (nBtnCount - 1) * m_btnHeightOffset));
    }

    m_bPhoto = bPhoto;
    if (bPhoto && !m_isBtnsFold) {
        m_flashlightUnfoldBtn->setVisible(true);
        m_filtersUnfoldBtn->setVisible(true);
        m_exposureBtn->setVisible(true);

        m_flashlightUnfoldBtn->setOpacity(102);
        m_filtersUnfoldBtn->setOpacity(102);
        m_exposureBtn->setOpacity(102);
    } else {
        m_flashlightUnfoldBtn->setVisible(false);
        m_filtersUnfoldBtn->setVisible(false);
        m_exposureBtn->setVisible(false);
    }
    update();
}

void takePhotoSettingAreaWidget::resizeScrollHeight(int height)
{
    m_scrollHeight = (m_filtersFoldBtn->height() + m_threeBtnOffset * 2) * 6 + height + 15; //Add a offset here
    if (m_scrollHeight > m_scrollArea->widget()->height())
        m_scrollHeight = m_scrollArea->widget()->height();
    m_scrollArea->resize(m_scrollArea->width(), m_scrollHeight);
    if (!m_scrollArea->isVisible())
        return;
    int updateHeight = (m_filtersFoldBtn->height() + m_threeBtnOffset*2 + 1) * 2 + m_scrollHeight;
    m_filtersCloseBtn->move(QPoint(5, (m_filtersFoldBtn->height() + m_threeBtnOffset*2) + m_scrollHeight));
    setFixedHeight(updateHeight);
    update();
}
