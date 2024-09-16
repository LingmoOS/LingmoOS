// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "aienhancefloatwidget.h"

#include <QVBoxLayout>

#include <DAnchors>
#include <DApplicationHelper>
#include <DGuiApplicationHelper>

const int FLOAT_WDITH = 72;
const int FLOAT_HEIGHT = 122;  // 172, 移除保存按钮
const int FLOAT_RIGHT_MARGIN = 15;
const int FLOAT_RADIUS = 18;
const QSize FLOAT_BTN_SIZE = QSize(40, 40);
const QSize FLOAT_ICON_SIZE = QSize(20, 20);

AIEnhanceFloatWidget::AIEnhanceFloatWidget(QWidget *parent)
    : DFloatingWidget(parent)
{
    setObjectName("AIEnhanceFloatWidget");
    setFixedSize(FLOAT_WDITH, FLOAT_HEIGHT);
    setFramRadius(FLOAT_RADIUS);
    setBlurBackgroundEnabled(true);
    initButtton();

    if (parent) {
        DAnchorsBase::setAnchor(this, Qt::AnchorRight, parent, Qt::AnchorRight);
        DAnchorsBase::setAnchor(this, Qt::AnchorVerticalCenter, parent, Qt::AnchorVerticalCenter);
        DAnchorsBase *anchor = DAnchorsBase::getAnchorBaseByWidget(this);
        if (anchor) {
            anchor->setRightMargin(FLOAT_RIGHT_MARGIN);
        }
    }

    onThemeChanged();
    connect(
        DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &AIEnhanceFloatWidget::onThemeChanged);
}

void AIEnhanceFloatWidget::initButtton()
{
    QVBoxLayout *backLayout = new QVBoxLayout(this);
    backLayout->setSpacing(0);
    backLayout->setContentsMargins(0, 0, 0, 0);

    bkgBlur = new DBlurEffectWidget(this);
    bkgBlur->setBlurRectXRadius(18);
    bkgBlur->setBlurRectYRadius(18);
    bkgBlur->setRadius(30);
    bkgBlur->setBlurEnabled(true);
    bkgBlur->setMode(DBlurEffectWidget::GaussianBlur);
    QColor maskColor(255, 255, 255, 76);
    bkgBlur->setMaskColor(maskColor);
    backLayout->addWidget(bkgBlur);

    QVBoxLayout *ctxLayout = new QVBoxLayout(bkgBlur);
    ctxLayout->setAlignment(Qt::AlignCenter);
    ctxLayout->setSpacing(10);

    resetBtn = new DIconButton(this);
    resetBtn->setFixedSize(FLOAT_BTN_SIZE);
    resetBtn->setIcon(QIcon::fromTheme("dcc_reset"));
    resetBtn->setIconSize(FLOAT_ICON_SIZE);
    resetBtn->setToolTip(tr("Reprovision"));
    ctxLayout->addWidget(resetBtn);
    connect(resetBtn, &DIconButton::clicked, this, &AIEnhanceFloatWidget::reset);

    // 屏蔽Save按钮
#if 0
    saveBtn = new DIconButton(this);
    saveBtn->setFixedSize(FLOAT_BTN_SIZE);
    saveBtn->setIcon(QIcon::fromTheme("dcc_save"));
    saveBtn->setIconSize(FLOAT_ICON_SIZE);
    saveBtn->setToolTip(tr("Save"));
    mainLayout->addWidget(saveBtn);
    connect(saveBtn, &DIconButton::clicked, this, &AIEnhanceFloatWidget::save);
#endif

    saveAsBtn = new DIconButton(this);
    saveAsBtn->setFixedSize(FLOAT_BTN_SIZE);
    saveAsBtn->setIcon(QIcon::fromTheme("dcc_file_save_as"));
    saveAsBtn->setIconSize(FLOAT_ICON_SIZE);
    saveAsBtn->setToolTip(tr("Save as"));
    ctxLayout->addWidget(saveAsBtn);
    connect(saveAsBtn, &DIconButton::clicked, this, &AIEnhanceFloatWidget::saveAs);

    setLayout(ctxLayout);
}

/**
   @brief 按主题更新界面，使得图像增强侧边栏和底栏效果一致
 */
void AIEnhanceFloatWidget::onThemeChanged()
{
    auto themeType = DGuiApplicationHelper::instance()->themeType();
    if (DGuiApplicationHelper::LightType == themeType) {
        QColor maskColor(247, 247, 247);
        maskColor.setAlphaF(0.15);
        bkgBlur->setMaskColor(maskColor);

        DPalette pa;
        pa = resetBtn->palette();
        pa.setColor(DPalette::Light, QColor("#FFFFFF"));
        pa.setColor(DPalette::Dark, QColor("#FFFFFF"));
        // 单个按钮边框
        QColor btnframecolor("#000000");
        btnframecolor.setAlphaF(0.00);
        pa.setColor(DPalette::FrameBorder, btnframecolor);
        // 取消阴影
        pa.setColor(DPalette::Shadow, btnframecolor);

        DApplicationHelper::instance()->setPalette(resetBtn, pa);
        DApplicationHelper::instance()->setPalette(saveAsBtn, pa);
    } else {
        QColor maskColor("#202020");
        maskColor.setAlphaF(0.50);
        bkgBlur->setMaskColor(maskColor);

        DPalette pa;
        pa = resetBtn->palette();
        pa.setColor(DPalette::Light, QColor("#303030"));
        pa.setColor(DPalette::Dark, QColor("#303030"));
        // 单个按钮边框
        QColor btnframecolor("#000000");
        btnframecolor.setAlphaF(0.30);
        pa.setColor(DPalette::FrameBorder, btnframecolor);
        // 取消阴影
        pa.setColor(DPalette::Shadow, btnframecolor);

        DApplicationHelper::instance()->setPalette(resetBtn, pa);
        DApplicationHelper::instance()->setPalette(saveAsBtn, pa);
    }
}
