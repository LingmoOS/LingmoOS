// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shottoolwidget.h"
#include "tooltips.h"
#include "../utils/configsettings.h"
#include "../utils.h"
#include "../accessibility/acTextDefine.h"
#include "../main_window.h"
#include "slider.h"

#include <DSlider>
#include <DLineEdit>
#include <DMenu>
#include <DFrame>
#include <DBlurEffectWidget>
#include <DPalette>

#include <QAction>
#include <QButtonGroup>
#include <QStyleFactory>
#include <QLine>
#include <QDebug>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

namespace {
//const int TOOLBAR_HEIGHT = 223;
//const int TOOLBAR_WIDTH = 40;
const QSize TOOL_ICON_SIZE = QSize(30, 30);
const QSize TOOL_BUTTON_SIZE = QSize(36, 36);
}

ShotToolWidget::ShotToolWidget(MainWindow *pmainwindow, DWidget *parent) :
    DStackedWidget(parent),
    m_thicknessBtnGroup(nullptr),
    m_pMainWindow(pmainwindow)
{
    initWidget();
}



void ShotToolWidget::initWidget()
{
//    this->setAttribute(Qt::WA_StyledBackground, true);
//    this->setStyleSheet("background-color: rgb(0,255, 0)");
//    setMinimumSize(TOOLBAR_WIDTH, TOOLBAR_HEIGHT);
    hintFilter = new HintFilter(this);
    initEffectLabel();
    m_textInitFlag = false;
    m_thicknessLabel = nullptr;
    m_thicknessInitFlag = false;
    m_textSubTool = nullptr;
}

void ShotToolWidget::installTipHint(QWidget *w, const QString &hintstr)
{
    // TODO: parent must be mainframe
    auto hintWidget = new ToolTips("", m_pMainWindow);
    hintWidget->hide();
    hintWidget->setText(hintstr);
//    hintWidget->setFixedHeight(32);
    w->setProperty("HintWidget", QVariant::fromValue<QWidget *>(hintWidget));
    w->installEventFilter(hintFilter);

}

//初始化模糊功能工具栏
void ShotToolWidget::initEffectLabel()
{
    m_effectSubTool = new DLabel(this);
    DBlurEffectWidget *t_blurArea = new DBlurEffectWidget(this);
    t_blurArea->setBlurRectXRadius(7);
    t_blurArea->setBlurRectYRadius(7);
    t_blurArea->setRadius(15);
    t_blurArea->setMode(DBlurEffectWidget::GaussianBlur);
    t_blurArea->setBlurEnabled(true);
    t_blurArea->setBlendMode(DBlurEffectWidget::InWidgetBlend);
    t_blurArea->setMaskColor(QColor(255, 255, 255, 0));

    QHBoxLayout *t_blurAreaLayout = new QHBoxLayout(this);
    t_blurAreaLayout->setMargin(0);

    //分割线
    DVerticalLine *t_seperator = new DVerticalLine(this);
    t_seperator->setFixedSize(3, 30);
    DVerticalLine *t_seperator0 = new DVerticalLine(this);
    t_seperator0->setFixedSize(3, 30);
    DVerticalLine *t_seperator1 = new DVerticalLine(this);
    t_seperator1->setFixedSize(3, 30);
    t_seperator1->hide();

    QButtonGroup *effectTypeBtnGroup = new QButtonGroup(this);
    effectTypeBtnGroup->setExclusive(true);

    ToolButton *mosaicBut = new ToolButton(this);
    mosaicBut->setFixedSize(TOOL_BUTTON_SIZE);
    mosaicBut->setIconSize(TOOL_ICON_SIZE);
    mosaicBut->setIcon(QIcon::fromTheme(QString("Mosaic_normal")));
    effectTypeBtnGroup->addButton(mosaicBut);
    ToolButton *blurBut = new ToolButton(this);
    blurBut->setFixedSize(TOOL_BUTTON_SIZE);
    blurBut->setIconSize(TOOL_ICON_SIZE);
    blurBut->setIcon(QIcon::fromTheme(QString("vague_normal")));
    effectTypeBtnGroup->addButton(blurBut);


    QButtonGroup *shapBtnGroup = new QButtonGroup(this);
    shapBtnGroup->setExclusive(true);
    ToolButton *rectBut = new ToolButton(this);
    rectBut->setFixedSize(TOOL_BUTTON_SIZE);
    rectBut->setIconSize(TOOL_ICON_SIZE);
    rectBut->setIcon(QIcon::fromTheme(QString("rectangle-normal")));
    installTipHint(rectBut, tr("Rectangle\nPress and hold Shift to draw a square"));
    shapBtnGroup->addButton(rectBut);
    ToolButton *ovalBut = new ToolButton(this);
    ovalBut->setFixedSize(TOOL_BUTTON_SIZE);
    ovalBut->setIconSize(TOOL_ICON_SIZE);
    ovalBut->setIcon(QIcon::fromTheme(QString("oval-normal")));
    installTipHint(ovalBut, tr("Ellipse\nPress and hold Shift to draw a circle"));
    shapBtnGroup->addButton(ovalBut);
    ToolButton *penBut = new ToolButton(this);
    penBut->setFixedSize(TOOL_BUTTON_SIZE);
    penBut->setIconSize(TOOL_ICON_SIZE);
    penBut->setIcon(QIcon::fromTheme(QString("effect_pen")));
    installTipHint(penBut, tr("Brush\nPress and hold Shift to draw a straight line"));
    shapBtnGroup->addButton(penBut);

    //模糊强度滑动条
    Slider *t_radiusSize = new Slider(Qt::Horizontal, this);
    t_radiusSize->slider()->setTickInterval(1);
    t_radiusSize->setMinimum(0);
    t_radiusSize->setMaximum(10);
    t_radiusSize->setLeftIcon(QIcon::fromTheme(QString("mosaic1")));
    t_radiusSize->setRightIcon(QIcon::fromTheme(QString("mosaic2")));
    t_radiusSize->setIconSize(QSize(16, 16));
    installTipHint(t_radiusSize, tr("Adjust blur strength (Scroll to adjust it)"));
    int t_fontSize = ConfigSettings::instance()->getValue("effect", "radius").toInt();
    t_radiusSize->setValue(t_fontSize);
    t_radiusSize->setMouseWheelEnabled(true);

    //笔刷大小滑动条
    Slider *t_lineWidthSize = new Slider(Qt::Horizontal, this);
    //t_lineWidthSize->setStyleSheet("border-image: url(:/icons/deepin/builtin/texts/vague_normal_32px.svg)");
    t_lineWidthSize->slider()->setTickInterval(1);
    t_lineWidthSize->setMinimum(0);
    t_lineWidthSize->setMaximum(10);
    t_lineWidthSize->slider()->setSingleStep(2);
    t_lineWidthSize->setLeftIcon(QIcon::fromTheme(QString("effect_pen_w")));
    //t_lineWidthSize->setRightIcon(QIcon::fromTheme(QString("effect_pen_w")));
    t_lineWidthSize->setIconSize(QSize(16, 16));
    installTipHint(t_lineWidthSize, tr("Adjust brush size (Scroll to adjust it)"));
    int t_width = ConfigSettings::instance()->getValue("effect", "line_width").toInt();
    t_lineWidthSize->setValue(t_width);
    t_lineWidthSize->setMouseWheelEnabled(true);

    DLabel *penWidth = new DLabel(this);
    penWidth->setElideMode(Qt::TextElideMode::ElideLeft);
    penWidth->setFixedSize(24, 24);
    DFontSizeManager::instance()->bind(penWidth, DFontSizeManager::T7);
    penWidth->setText(QString("%1").arg((t_width + 1) * 2));

    QHBoxLayout *widthSizeLayout = new QHBoxLayout(this);
    widthSizeLayout->setContentsMargins(0, 0, 0, 0);
    widthSizeLayout->addWidget(t_lineWidthSize);
    widthSizeLayout->addWidget(penWidth);

    penWidth->hide();
    t_lineWidthSize->hide();

    t_blurAreaLayout->addWidget(mosaicBut);
    t_blurAreaLayout->addWidget(blurBut);
    t_blurAreaLayout->addWidget(t_seperator);
    t_blurAreaLayout->addWidget(rectBut);
    t_blurAreaLayout->addWidget(ovalBut);
    t_blurAreaLayout->addWidget(penBut);
    t_blurAreaLayout->addWidget(t_seperator0);
    t_blurAreaLayout->addWidget(t_radiusSize);
    t_blurAreaLayout->addWidget(t_seperator1);
    t_blurAreaLayout->addLayout(widthSizeLayout);
    t_blurArea->setLayout(t_blurAreaLayout);

    bool isBlur = ConfigSettings::instance()->getValue("effect", "isBlur").toBool();
    if (isBlur) {
        blurBut->setChecked(true);
        t_radiusSize->setLeftIcon(QIcon::fromTheme(QString("effect_pen1")));
        t_radiusSize->setRightIcon(QIcon::fromTheme(QString("effect_pen2")));
    } else {
        mosaicBut->setChecked(true);
        t_radiusSize->setLeftIcon(QIcon::fromTheme(QString("mosaic1")));
        t_radiusSize->setRightIcon(QIcon::fromTheme(QString("mosaic2")));
    }

    int isOvalBut = ConfigSettings::instance()->getValue("effect", "isOval").toInt();

    if (isOvalBut == 0) {
        ovalBut->setChecked(true);
    } else if (isOvalBut == 1) {
        rectBut->setChecked(true);
    } else {
        penBut->setChecked(true);
        t_lineWidthSize->show();
        penWidth->show();
        t_seperator1->show();
    }

    connect(effectTypeBtnGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),
    [ = ] {
        ConfigSettings::instance()->setValue("effect", "isBlur", blurBut->isChecked());
        if (blurBut->isChecked()) {
            t_radiusSize->setLeftIcon(QIcon::fromTheme(QString("effect_pen1")));
            t_radiusSize->setRightIcon(QIcon::fromTheme(QString("effect_pen2")));
        } else {
            t_radiusSize->setLeftIcon(QIcon::fromTheme(QString("mosaic1")));
            t_radiusSize->setRightIcon(QIcon::fromTheme(QString("mosaic2")));
        }
    });


    connect(shapBtnGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),
    [ = ] {
        int value = 2;
        if (rectBut->isChecked()) {
            value = 1;
        } else if (ovalBut->isChecked()) {
            value = 0;
        }

        if (penBut->isChecked()) {
            t_lineWidthSize->show();
            penWidth->show();
            t_seperator1->show();
        } else {
            t_lineWidthSize->hide();
            penWidth->hide();
            t_seperator1->hide();
        }
        ConfigSettings::instance()->setValue("effect", "isOval", value);
    });


    connect(t_radiusSize, &DSlider::valueChanged, this, [ = ] {
        int t_value = t_radiusSize->value();
        ConfigSettings::instance()->setValue("effect", "radius", t_value);
    });

    connect(t_lineWidthSize, &DSlider::valueChanged, this, [ = ] {
        int t_value = t_lineWidthSize->value();
        penWidth->setText(QString("%1").arg((t_value + 1) * 2));
        ConfigSettings::instance()->setValue("effect", "line_width", t_value);
    });

    QHBoxLayout *rectLayout = new QHBoxLayout(this);
    rectLayout->setMargin(0);

    rectLayout->addWidget(t_blurArea);
    m_effectSubTool->setLayout(rectLayout);

    addWidget(m_effectSubTool);
}

//初始化文字工具工具栏
void ShotToolWidget::initTextLabel()
{
    qDebug() << ">>>>>>>> func: " << __FUNCTION__;
    m_textSubTool = new DLabel(this);

    DBlurEffectWidget *t_blurArea = new DBlurEffectWidget(this);
    t_blurArea->setBlurRectXRadius(7);
    t_blurArea->setBlurRectYRadius(7);
    t_blurArea->setRadius(15);
    t_blurArea->setMode(DBlurEffectWidget::GaussianBlur);
    t_blurArea->setBlurEnabled(true);
    t_blurArea->setBlendMode(DBlurEffectWidget::InWidgetBlend);
    t_blurArea->setMaskColor(QColor(255, 255, 255, 0));
    //t_blurArea->setFixedSize(TOOL_SLIDERBlUR_SIZE);

    QHBoxLayout *t_blurAreaLayout = new QHBoxLayout(this);
    Slider *t_textFontSize = new Slider(Qt::Horizontal, this);
    t_textFontSize->slider()->setTickInterval(1);
    //t_textFontSize->setFixedSize(TOOL_SLIDER_SIZE);
    t_textFontSize->setMinimum(0);
    t_textFontSize->setMaximum(10);
    t_textFontSize->setLeftIcon(QIcon::fromTheme("Aa small_normal"));
    t_textFontSize->setRightIcon(QIcon::fromTheme("Aa big_normal"));
    t_textFontSize->setIconSize(TOOL_ICON_SIZE);
    t_textFontSize->setMouseWheelEnabled(true);
    installTipHint(t_textFontSize, tr("Adjust text size (Scroll to adjust it)"));

    static const int indexTofontsize[11] = {9, 10, 12, 14, 18, 24, 36, 48, 64, 72, 96};
    int t_fontSize = ConfigSettings::instance()->getValue("text", "fontsize").toInt();
    for (int i = 0; i < 11; ++i) {
        if (indexTofontsize[i] == t_fontSize) {
            t_textFontSize->setValue(i);
            break;
        }
    }
    t_blurAreaLayout->setContentsMargins(0, 0, 0, 0);
    t_blurAreaLayout->addWidget(t_textFontSize);
    t_blurArea->setLayout(t_blurAreaLayout);

    connect(t_textFontSize, &DSlider::valueChanged, this, [ = ] {
        int t_value = t_textFontSize->value();
        ConfigSettings::instance()->setValue("text", "fontsize", indexTofontsize[t_value]);
    });

    QHBoxLayout *rectLayout = new QHBoxLayout(this);
    rectLayout->setMargin(3);

    rectLayout->addWidget(t_blurArea);

    m_textSubTool->setLayout(rectLayout);
}

void ShotToolWidget::initThicknessLabel()
{
    m_thicknessLabel = new DLabel(this);
    //粗细按钮组
    m_thicknessBtnGroup = new QButtonGroup(this);
    m_thicknessBtnGroup->setExclusive(true);

    //水平布局
    QHBoxLayout *hBox = new QHBoxLayout(m_thicknessLabel);
    hBox->setContentsMargins(0, 0, 0, 0);

    //根据三个粗细按钮名称创建对应的控件
    QStringList thicknessBtnName;
    thicknessBtnName << "small" << "medium" << "big";
    for (int i = 0; i < thicknessBtnName.size(); i++) {
        ToolButton *thicknessBtn = new ToolButton(this);
        Utils::setAccessibility(thicknessBtn, QString("%1_thicknessBtn").arg(thicknessBtnName[i]));
        thicknessBtn->setFixedSize(TOOL_BUTTON_SIZE);
        thicknessBtn->setIconSize(TOOL_ICON_SIZE);
        thicknessBtn->setIcon(QIcon::fromTheme(QString("brush %1_normal").arg(thicknessBtnName[i])));
        m_thicknessBtnGroup->addButton(thicknessBtn);
        m_thicknessBtnGroup->setId(thicknessBtn, i);
        hBox->addWidget(thicknessBtn);
    }

    //点击粗细按钮组中对应的按钮触发写到配置文件的操作
    connect(m_thicknessBtnGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),
    [ = ](QAbstractButton * button) {
        ToolButton *thicknessBtn = static_cast<ToolButton *>(button) ;
        if (thicknessBtn->isChecked()) {
            thicknessBtn->update();
            ConfigSettings::instance()->setValue(m_shapeName, "line_width", m_thicknessBtnGroup->id(thicknessBtn));
        }
    });
    m_thicknessLabel->setLayout(hBox);
    addWidget(m_thicknessLabel);
}

void ShotToolWidget::switchContent(QString shapeType)
{
    m_shapeName = shapeType;
    if (!shapeType.isEmpty()) {
        //矩形、圆形、直线、箭头、画笔会加载粗细调整面板
        if (shapeType == "rectangle" ||
                shapeType == "oval" ||
                shapeType == "line" ||
                shapeType == "arrow" ||
                shapeType == "pen") {
            if (m_textSubTool)
                removeWidget(m_textSubTool);
            if (m_thicknessInitFlag == false) {
                initThicknessLabel();
                m_thicknessInitFlag = true;
            }
            int linewidth_index  = -1;
            linewidth_index = ConfigSettings::instance()->getValue(m_shapeName, "line_width").toInt();
            if (linewidth_index != -1)
                m_thicknessBtnGroup->button(linewidth_index)->click();
            addWidget(m_thicknessLabel);
            setCurrentWidget(m_thicknessLabel);
        }
        //文本会加载字体调整面板
        else if (shapeType == "text") {
            if (m_thicknessLabel)
                removeWidget(m_thicknessLabel);
            if (m_textInitFlag == false) {
                initTextLabel();
                m_textInitFlag = true;
            }
            addWidget(m_textSubTool);
            setCurrentWidget(m_textSubTool);
        }
        //模糊会加载模糊调整面板
        else if (shapeType == "effect") {
            setCurrentWidget(m_effectSubTool);
        }
    }
}

void ShotToolWidget::colorChecked(QString colorType)
{
    Q_UNUSED(colorType);
    //if(m_currentType == "rectangle"){
//    if (m_blurRectButton && m_blurRectButton->isChecked())
//        m_blurRectButton->click();
//    if (m_mosaicRectButton && m_mosaicRectButton->isChecked())
//        m_mosaicRectButton->click();
//    //}else if(m_currentType == "oval"){
//    if (m_blurCircButton && m_blurCircButton->isChecked())
//        m_blurCircButton->click();
//    if (m_mosaicCircButton && m_mosaicCircButton->isChecked())
//        m_mosaicCircButton->click();
//    // }
}
ShotToolWidget::~ShotToolWidget()
{
    if (nullptr != hintFilter) {
        delete hintFilter;
        hintFilter = nullptr;
    }
}
