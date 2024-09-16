// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "drivericonwidget.h"

#include <DBlurEffectWidget>
#include <DWidget>
#include <DLabel>
#include <DWaterProgress>
#include <DFontSizeManager>
#include <DApplicationHelper>
#include <DRadioButton>
#include <DGuiApplicationHelper>
#include <DLabel>
#include <DCheckBox>
#include <DFrame>

#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE
/**
 * @brief palrtteTransparency 设置widget的字体颜色
 * @param alphaFloat 增加的透明度，如alphaFloat=-10：透明度90（100 - 10）
 */
static void palrtteTransparency(QWidget *widget, qint8 alphaFloat)
{
    QPalette palette = widget->palette();
    QColor color = DGuiApplicationHelper::adjustColor(palette.color(QPalette::Active, QPalette::BrightText), 0, 0, 0, 0, 0, 0, alphaFloat);
    palette.setColor(QPalette::WindowText, color);
    widget->setPalette(palette);
}

DriverIconWidget::DriverIconWidget(const QString &strTitle, const QString &strDesc, QWidget *parent)
    : DWidget(parent)
{
    iconWidget = new DWaterProgress;
    iconWidget->setFixedSize(100, 100);
    iconWidget->setValue(50);
    iconWidget->start();

    initUI(iconWidget, strTitle, strDesc);
}

DriverIconWidget::DriverIconWidget(const QPixmap &pixmap, const QString &strTitle, const QString &strDesc, QWidget *parent)
    : DWidget(parent)
{
    DLabel *iconWidget = new DLabel;
    iconWidget->setFixedSize(100, 100);
    iconWidget->setPixmap(pixmap);
    iconWidget->setScaledContents(true);

    initUI(iconWidget, strTitle, strDesc);
}

DriverIconWidget::~DriverIconWidget()
{

}

void DriverIconWidget::initUI(QWidget *iconWidget, const QString &strTitle, const QString &strDesc)
{
    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->setContentsMargins(0, 0, 0, 0);

    QSpacerItem *verticalSpacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);
    vlayout->addItem(verticalSpacer);
    vlayout->addStretch();
    vlayout->addWidget(iconWidget, 0, Qt::AlignCenter);
    vlayout->addStretch();
    //strTitle
    {
        DLabel *label = new DLabel(strTitle);
        label->setFixedWidth(440);
        label->setMinimumHeight(20);
        label->setAlignment(Qt::AlignCenter);
        label->setWordWrap(true);
        label->adjustSize();//换行，导致长提示语换行，显示不全
        DFontSizeManager::instance()->bind(label, DFontSizeManager::T7, QFont::Medium);

        //透明度90
        palrtteTransparency(label, -10);
        QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [label]() {
            //透明度90
            palrtteTransparency(label, -10);
        });

        vlayout->addWidget(label, 0, Qt::AlignCenter);
    }
    //strDesc
    {
        DLabel *label = new DLabel(strDesc);
        label->setFixedWidth(440);
        label->setMinimumHeight(20);
        label->setAlignment(Qt::AlignCenter);
        DFontSizeManager::instance()->bind(label, DFontSizeManager::T7, QFont::Medium);

        QFontMetrics elideFont(label->font());
        label->setText(elideFont.elidedText(strDesc, Qt::ElideRight, label->width()));
        label->setToolTip(strDesc);

        //透明度50
        palrtteTransparency(label, -30);
        QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [label]() {
            //透明度50
            palrtteTransparency(label, -30);
        });

        vlayout->addWidget(label, 0, Qt::AlignCenter);
    }
    vlayout->addSpacing(10);
    vlayout->addStretch();

    setLayout(vlayout);
}
