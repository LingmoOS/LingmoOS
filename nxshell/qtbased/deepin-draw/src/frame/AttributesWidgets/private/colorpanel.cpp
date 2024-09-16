// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "colorpanel.h"

#include <DGuiApplicationHelper>

#include <QPainter>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QButtonGroup>
#include <QDebug>
#include <QRegExpValidator>

#include "utils/global.h"
#include "utils/baseutils.h"
#include "drawshape/cdrawparamsigleton.h"
#include "frame/cviewmanagement.h"
#include "frame/cgraphicsview.h"

#include "ciconbutton.h"
#include "colorlabel.h"
#include "colorslider.h"
#include "pickcolorwidget.h"
#include "calphacontrolwidget.h"
#include "AttributesWidgets/ccolorpickwidget.h"
#include "application.h"

#include <DWindowManagerHelper>

DGUI_USE_NAMESPACE

const int ORGIN_WIDTH = 314;
const int PANEL_WIDTH = 294;
const int ORIGIN_HEIGHT = 250;
const int EXPAND_HEIGHT = 475;
const int RADIUS = 8;
const QSize COLOR_BORDER_SIZE = QSize(34, 34);

bool ColorPanel::s_expand = false;
ColorButton::ColorButton(const QColor &color, DWidget *parent)
    : QPushButton(parent)
    , m_color(color)
{
    setFixedSize(COLOR_BORDER_SIZE);
    setCheckable(true);

    connect(this, &ColorButton::clicked, this, [ = ]() {
        this->setChecked(true);
    });
}

void ColorButton::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setBrush(QBrush(m_color));
    QColor colorTrasparent(Qt::white);
    colorTrasparent.setAlpha(0);

    QPen pen;
    pen.setWidth(1);
    if (m_color == colorTrasparent) {
        pen.setColor(QColor("#cccccc"));
    } else {
        pen.setColor(QColor(0, 0, 0, 55));
    }
    painter.setPen(pen);
    painter.drawRoundedRect(QRect(3, 3, this->width() - 6,
                                  this->height() - 6),
                            RADIUS, RADIUS);
    if (m_color == colorTrasparent) {
        pen.setColor(QColor("#ff804d"));
        painter.setPen(pen);
        painter.drawLine(6, this->height() - 6, this->width() - 6, 6);
    }

    if (isChecked()) {
        painter.setBrush(QBrush());
        QPen borderPen;
        borderPen.setWidth(2);
        borderPen.setColor("#008eff");
        painter.setPen(borderPen);
        painter.drawRoundedRect(QRect(1, 1, this->width() - 2,
                                      this->height() - 2),
                                RADIUS, RADIUS);
    }
}

QColor ColorButton::color()
{
    return m_color;
}

ColorButton::~ColorButton()
{
}

ColorPanel::ColorPanel(DWidget *parent)
    : DWidget(parent)
{
    initUI();
    initConnection();
}

ColorPanel::~ColorPanel()
{
}

void ColorPanel::setTheme(int theme)
{
    int themeType = theme;
    m_colorfulBtn->setTheme(themeType);
    m_pickColWidget->setTheme(themeType);
}

void ColorPanel::showEvent(QShowEvent *event)
{
    //显示时为了让光标不在编辑框内
    setFocus();
    DWidget::showEvent(event);
}

void ColorPanel::initUI()
{
    DWidget *colorBtnWidget = new DWidget(this);
    colorBtnWidget->setFixedSize(ORGIN_WIDTH, ORIGIN_HEIGHT);

    m_colList = specifiedColorList();

    m_colorsButtonGroup = new QButtonGroup(this);
    m_colorsButtonGroup->setExclusive(true);

    QGridLayout *gLayout = new QGridLayout;
    gLayout->setVerticalSpacing(4);
    gLayout->setHorizontalSpacing(4);

    for (int i = 0; i < m_colList.length(); i++) {
        ColorButton *cb = new ColorButton(m_colList[i], this);
        setWgtAccesibleName(cb, QString("Panel %1 pushbutton").arg(cb->color().name()));
        cb->setFocusPolicy(Qt::NoFocus);
        m_cButtonList.append(cb);
        gLayout->addWidget(cb, i / 8, i % 8);
        m_colorsButtonGroup->addButton(cb, i);
    }

    m_alphaControlWidget = new CAlphaControlWidget(this);
    m_alphaControlWidget->setObjectName("CAlphaControlWidget");
    m_alphaControlWidget->setFocusPolicy(Qt::NoFocus);

    DWidget *colorValueWidget = new DWidget(this);
    colorValueWidget->setFocusPolicy(Qt::NoFocus);
    colorValueWidget->setFixedWidth(PANEL_WIDTH);
    DLabel *colLabel = new DLabel(colorValueWidget);
    QFont colLabelFont = colLabel->font();
    colLabelFont.setPixelSize(13);
    colLabel->setFixedWidth(35);
    colLabel->setText(tr("Color"));
    colLabel->setFont(colLabelFont);

    m_colLineEdit = new DLineEdit(colorValueWidget);
    m_colLineEdit->setObjectName("ColorLineEdit");
    m_colLineEdit->setFixedSize(180, 36);
    m_colLineEdit->setClearButtonEnabled(false);
    m_colLineEdit->lineEdit()->setValidator(new QRegExpValidator(QRegExp("[0-9A-Fa-f]{6}"), this));
    m_colLineEdit->setText("ffffff");
    connect(m_colLineEdit, &DLineEdit::returnPressed, this, [ = ]() {
        this->parentColorWidget()->hide();
    });
    //m_colLineEdit->lineEdit()->setFocusPolicy(Qt::NoFocus);

    QMap<int, QMap<CIconButton::EIconButtonSattus, QString>> pictureMap;
    pictureMap[DGuiApplicationHelper::LightType][CIconButton::Normal] = QString(":/theme/light/images/draw/palette_normal.svg");
    pictureMap[DGuiApplicationHelper::LightType][CIconButton::Hover] = QString(":/theme/light/images/draw/palette_normal.svg");
    pictureMap[DGuiApplicationHelper::LightType][CIconButton::Press] = QString(":/theme/light/images/draw/palette_normal.svg");
    pictureMap[DGuiApplicationHelper::LightType][CIconButton::Active] = QString(":/theme/light/images/draw/palette_normal.svg");

    pictureMap[DGuiApplicationHelper::DarkType][CIconButton::Normal] = QString(":/theme/dark/images/draw/palette_normal.svg");
    pictureMap[DGuiApplicationHelper::DarkType][CIconButton::Hover] = QString(":/theme/dark/images/draw/palette_normal.svg");
    pictureMap[DGuiApplicationHelper::DarkType][CIconButton::Press] = QString(":/theme/dark/images/draw/palette_normal.svg");
    pictureMap[DGuiApplicationHelper::DarkType][CIconButton::Active] = QString(":/theme/dark/images/draw/palette_normal.svg");

    m_colorfulBtn = new CIconButton(pictureMap, QSize(55, 36), colorValueWidget, false);
    m_colorfulBtn->setObjectName("CIconButton");
    m_colorfulBtn->setFocusPolicy(Qt::NoFocus);

    QHBoxLayout *colorLayout = new QHBoxLayout(colorValueWidget);
    colorLayout->setMargin(0);
    colorLayout->setSpacing(0);
    colorLayout->addWidget(colLabel);
    colorLayout->addSpacing(14);
    colorLayout->addWidget(m_colLineEdit);
    colorLayout->addSpacing(10);
    colorLayout->addWidget(m_colorfulBtn);

    m_pickColWidget = new PickColorWidget(this);
    m_pickColWidget->setObjectName("PickColorWidget");
    m_pickColWidget->setFocusPolicy(Qt::NoFocus);

    QVBoxLayout *vLayout = new QVBoxLayout(colorBtnWidget);
    vLayout->setSpacing(0);
    vLayout->addSpacing(10);
    vLayout->addLayout(gLayout);
    vLayout->addSpacing(10);
    vLayout->addWidget(m_alphaControlWidget, 0, Qt::AlignCenter);
    vLayout->addSpacing(10);
    vLayout->addWidget(colorValueWidget, 0, Qt::AlignCenter);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(colorBtnWidget);
    layout->addWidget(m_pickColWidget, 0, Qt::AlignCenter);

    this->updateExpendArea();
}

void ColorPanel::initConnection()
{
    //1.颜色按钮组
    connect(m_colorsButtonGroup, QOverload<QAbstractButton *, bool>::of(&QButtonGroup::buttonToggled),
    this, [ = ](QAbstractButton * pBtn, bool checked) {
        ColorButton *pColorBtn = qobject_cast<ColorButton *>(pBtn);
        if (checked) {
            this->setColor(pColorBtn->color());
        }
    });

    //2.pick板颜色设置完成
    connect(m_pickColWidget, &PickColorWidget::colorChanged, this, [ = ](const QColor & color) {
        this->setColor(color);
    });

    //3.pick板颜色预览
    connect(m_pickColWidget, &PickColorWidget::previewedColorChanged, [ = ](const QColor & color) {
        this->setColor(color, true, EChangedUpdate);
    });

    //4.lineedit颜色设置
    connect(m_colLineEdit, &DLineEdit::textChanged, this, [ = ](const QString & colorStr) {
        if (colorStr.size() == 6) {
            QColor c("#" + colorStr);
            if (c.isValid()) {
                this->setColor(c, true, EChanged);
            }
        }
    });
//    connect(m_colLineEdit, &DLineEdit::editingFinished, this, [ = ]() {
//        qDebug() << "DLineEdit::editingFinished ------- ";
//        QString colorStr = m_colLineEdit->text();
//        if (colorStr.size() == 6) {
//            QColor c("#" + colorStr);
//            if (c.isValid()) {
//                int alpha = m_alphaControlWidget->alpha();
//                c.setAlpha(alpha);
//                this->setColor(c, true, EChanged);
//            }
//        }
//    });

    //5.设置透明度
    connect(m_alphaControlWidget, &CAlphaControlWidget::alphaChanged, this, [ = ](int alp, EChangedPhase phase) {
        QColor c = color();
        if (!c.isValid()) {
            c = QColor(0, 0, 0, alp);
        } else {
            c.setAlpha(alp);
        }
        qDebug() << "alphaChanged apl = " << c.alpha();
        this->setColor(c, true, phase);
    });

    //展开按钮
    connect(m_colorfulBtn, &CIconButton::buttonClick, this, [ = ] {
        s_expand = !s_expand;
        this->updateExpendArea();
    });
}

CColorPickWidget *ColorPanel::parentColorWidget()
{
    return qobject_cast<CColorPickWidget *>(parent());
}

void ColorPanel::setColor(const QColor &c,
                          bool internalChanged,
                          EChangedPhase phase)
{
    if (phase == EChangedFinished || phase == EChanged)
        curColor = c;

    updateColor(c);

    if (internalChanged) {
        emit colorChanged(c, phase);
    }
}

QColor ColorPanel::color()
{
    return curColor;
}

//DLineEdit *ColorPanel::colorLineEditor()
//{
//    return m_colLineEdit;
//}

void ColorPanel::setExpandWidgetVisble(bool visble)
{
    s_expand = visble;
    updateExpendArea();
}

void ColorPanel::updateColor(const QColor &previewColor)
{
    //BUG118771 https://pms.uniontech.com/bug-view-118771.html
    //QColor c = previewColor.isValid() ? previewColor : curColor;
    QColor c = previewColor.isValid() ? previewColor : (curColor.isValid() ? curColor : previewColor);

    //1.检查当前颜色是否是颜色组的颜色值
    int id = m_colList.indexOf(/*curColor*/QColor(curColor.red(), curColor.green(), curColor.blue()));
    if (curColor.alpha() == 0) {
        id = 0;
    }
    if (id != -1) {
        m_colorsButtonGroup->blockSignals(true);
        m_colorsButtonGroup->button(id)->setChecked(true);
        m_colorsButtonGroup->blockSignals(false);
    } /*else {
        //m_colorsButtonGroup->
        if (curColor.alpha() == 0) {
            m_colorsButtonGroup->blockSignals(true);
            m_colorsButtonGroup->button(0)->setChecked(true);
            m_colorsButtonGroup->blockSignals(false);
        }
    }*/

    //2.pick界面刷新设置为该颜色
    m_pickColWidget->setColor(c, false);

    //3.设置当前颜色的透明度(被动设置改值)
    m_alphaControlWidget->setAlpha(c.alpha(), false);

    //4.更新颜色名字
    QString colorName = "";
    if (c.name().contains("#")) {
        colorName = c.name().split("#").last();
    }
    //qDebug() << "colorName ====== " << colorName << "alpha = " << c.alpha() << "curColor ap = " << curColor.alpha();
    m_colLineEdit->blockSignals(true);
    m_colLineEdit->setText(colorName);
    m_colLineEdit->blockSignals(false);
}

void ColorPanel::updateExpendArea()
{
    if (!s_expand) {
        m_pickColWidget->hide();
        setFixedHeight(ORIGIN_HEIGHT);
        updateGeometry();
    } else {
        m_pickColWidget->show();
        setFixedHeight(EXPAND_HEIGHT);
        updateGeometry();
    }

    if (this->parentColorWidget() != nullptr && DWindowManagerHelper::instance()->hasBlurWindow()) {
        this->parentColorWidget()->setContent(this);
    }
}
