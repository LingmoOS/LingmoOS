// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <DIconButton>
#include <QButtonGroup>
#include <DGuiApplicationHelper>

#include "rectradiusstylewidget.h"
#include "cspinbox.h"
#include "globaldefine.h"
#include "boxlayoutwidget.h"
#include "globaldefine.h"
#include "drawboard.h"
#include "pagescene.h"

RectRadiusStyleWidget::RectRadiusStyleWidget(DrawBoard *drawBoard, QWidget *parent) : AttributeWgt(ERectRadius, parent), m_drawboard(drawBoard)
{
    initUi();
    initConnect();
}

void RectRadiusStyleWidget::setVar(const QVariant &var)
{

    QVariantList l = var.toList();
    if (l.isEmpty()) {
        bool rs = false;
        l <<  var.toInt(&rs);
        if (!rs) {
            m_radius->setSpecialText(SPECIAL_TEXT);
            m_left->setSpecialText(SPECIAL_TEXT);
            m_right->setSpecialText(SPECIAL_TEXT);
            m_leftBottom->setSpecialText(SPECIAL_TEXT);
            m_rightBottom->setSpecialText(SPECIAL_TEXT);
            QSignalBlocker block(m_radiusSlider);
            m_radiusSlider->setValue(0);
            return;
        }
    }

    int i = 0;
    if (1 == l.size()) {
        setSpinBoxValue(m_radius, l.at(i).toInt());
        setSpinBoxValue(m_left, l.at(i).toInt());
        setSpinBoxValue(m_right, l.at(i).toInt());
        setSpinBoxValue(m_leftBottom, l.at(i).toInt());
        setSpinBoxValue(m_rightBottom, l.at(i).toInt());
        showByChecked();
    } else {
        setSpinBoxValue(m_left, l.at(i++).toInt());
        setSpinBoxValue(m_right, l.at(i++).toInt());
        setSpinBoxValue(m_leftBottom, l.at(i++).toInt());
        setSpinBoxValue(m_rightBottom, l.at(i++).toInt());
        showByChecked(true);
    }

    QSignalBlocker block(m_radiusSlider);
    m_radiusSlider->setValue(m_left->value());

    int minValue = m_left->value();
    setSpinBoxValue(m_radius, minValue);
}

void RectRadiusStyleWidget::initUi()
{
    setFocusPolicy(Qt::NoFocus);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);

    DLabel *label = new DLabel(tr("Rounded corners"), this);

    m_sameRadiusButton = new DToolButton(nullptr);
    m_sameRadiusButton->setIcon(QIcon::fromTheme("unified"));
    m_sameRadiusButton->setToolTip(tr("Same radius"));
    setWgtAccesibleName(m_sameRadiusButton, "Same_radius");
    m_sameRadiusButton->setMinimumSize(38, 38);
    m_sameRadiusButton->setIconSize(QSize(24, 24));
    m_sameRadiusButton->setFocusPolicy(Qt::NoFocus);
    m_sameRadiusButton->setCheckable(true);

    m_diffRadiusButton = new DToolButton(nullptr);
    m_diffRadiusButton->setIcon(QIcon::fromTheme("part"));
    m_diffRadiusButton->setToolTip(tr("Different radii"));
    setWgtAccesibleName(m_diffRadiusButton, "Different_radius");
    m_diffRadiusButton->setMinimumSize(38, 38);
    m_diffRadiusButton->setIconSize(QSize(24, 24));
    m_diffRadiusButton->setFocusPolicy(Qt::NoFocus);
    m_diffRadiusButton->setCheckable(true);

    m_checkgroup = new QButtonGroup;
    m_checkgroup->setExclusive(true);
    m_checkgroup->addButton(m_sameRadiusButton);
    m_checkgroup->addButton(m_diffRadiusButton);

    QHBoxLayout *titleLayout = new QHBoxLayout;
    titleLayout->addWidget(label, 8);
    titleLayout->addWidget(m_sameRadiusButton, 1);
    titleLayout->addWidget(m_diffRadiusButton, 1);

    m_diffRadiusWidget = new BackgroundWidget;

    bool   darkTheme = false;
#ifdef USE_DTK
    darkTheme = (DGuiApplicationHelper::instance()->themeType()  == 2);
#endif
    QColor color  = !darkTheme ? QColor(211, 211, 211) : QColor(0, 0, 0, int(0.1 * 255));

    m_diffRadiusWidget->setBackgroudColor(color);
    m_diffRadiusWidget->setFocusPolicy(Qt::NoFocus);

    QGridLayout *angleLayout = new QGridLayout;
    angleLayout->setContentsMargins(0, 10, 0, 10);
    angleLayout->setMargin(10);
    angleLayout->setVerticalSpacing(10);
    angleLayout->setHorizontalSpacing(10);

    const int MaxWidth = 120;
    const int MaxValue = 1000;

    m_left = new CSpinBox(this);
    m_left->setToolTip(tr(""));
    setWgtAccesibleName(m_left, "left_radius");
    m_left->setMaximumWidth(MaxWidth);
    m_left->setSpinRange(0, 1000);
    m_left->setEnabledEmbedStyle(true);
    m_left->setValueChangedKeepFocus(true);

    m_right = new CSpinBox(this);
    m_right->setToolTip(tr(""));
    setWgtAccesibleName(m_right, "right_radius");
    m_right->setMaximumWidth(MaxWidth);
    m_right->setSpinRange(0, MaxValue);
    m_right->setEnabledEmbedStyle(true);
    m_right->setValueChangedKeepFocus(true);

    m_leftBottom = new CSpinBox(this);
    m_leftBottom->setToolTip(tr(""));
    setWgtAccesibleName(m_leftBottom, "left_bottom_radius");
    m_leftBottom->setMaximumWidth(MaxWidth);
    m_leftBottom->setSpinRange(0, MaxValue);
    m_leftBottom->setEnabledEmbedStyle(true);
    m_leftBottom->setValueChangedKeepFocus(true);

    m_rightBottom = new CSpinBox(this);
    m_rightBottom->setToolTip(tr(""));
    setWgtAccesibleName(m_rightBottom, "right bottom radius");
    m_rightBottom->setMaximumWidth(MaxWidth);
    m_rightBottom->setSpinRange(0, MaxValue);
    m_rightBottom->setEnabledEmbedStyle(true);
    m_rightBottom->setValueChangedKeepFocus(true);

    m_diffRadiusWidget->setLayout(angleLayout);
    angleLayout->addWidget(m_left, 0, 0);
    angleLayout->addWidget(m_right, 0, 1);
    angleLayout->addWidget(m_leftBottom, 1, 0);
    angleLayout->addWidget(m_rightBottom, 1, 1);
    mainLayout->addLayout(titleLayout);
    mainLayout->addWidget(m_diffRadiusWidget);

    m_sameRadiusWidget = new BoxLayoutWidget(this);
    m_sameRadiusWidget->setMargins(8);
    m_sameRadiusWidget->setBackgroundRect(true);
    m_sameRadiusWidget->setFrameRect(false);
    m_radiusSlider = new DSlider;
    m_radiusSlider->setMaximum(1000);

    m_radius = new CSpinBox();
    m_radius->setToolTip(tr("Radius"));
    setWgtAccesibleName(m_radius, "radius");
    m_radius->setSpinRange(0, 1000);
    m_radius->setEnabledEmbedStyle(true);

    m_sameRadiusWidget->addWidget(m_radiusSlider, 3);
    m_sameRadiusWidget->addWidget(m_radius, 1);
    mainLayout->addWidget(m_sameRadiusWidget);

    m_sameRadiusButton->setChecked(true);
    m_sameRadiusWidget->show();
    m_diffRadiusWidget->hide();
    this->setLayout(mainLayout);
    addHSeparateLine();
}

void RectRadiusStyleWidget::initConnect()
{
    connect(m_checkgroup, qOverload<QAbstractButton *>(&QButtonGroup::buttonPressed), this, [ = ](QAbstractButton * button) {
        if (button->isChecked()) {
            return;
        }
        showByChecked(button != m_sameRadiusButton);
        int value = 0;
        if (button == m_sameRadiusButton) {//不等模式切换相等模式
            value = m_left->value() < value ? value : m_left->value();
        } else {
            //相等模式切换不等模式
            value = m_radiusSlider->value();
        }

        m_radius->setValue(value);
        m_radiusSlider->setValue(value);
        m_left->setValue(value);
        m_right->setValue(value);
        m_leftBottom->setValue(value);
        m_rightBottom->setValue(value);

        emitValueChange(m_left->value(), m_right->value(), m_leftBottom->value(), m_rightBottom->value(), EChanged);
    });

    connect(m_radiusSlider, &DSlider::sliderPressed, this, [ = ]() {
        int value = m_radiusSlider->value();
        setSpinBoxValue(m_radius, value);
        emitValueChange(value, value, value, value, EChangedBegin);
    });

    connect(m_radiusSlider, &DSlider::valueChanged, this, [ = ](int value) {
        setSpinBoxValue(m_radius, value);
        emitValueChange(value, value, value, value, EChangedUpdate);

    });
    connect(m_radiusSlider, &DSlider::sliderReleased, this, [ = ]() {
        int value = m_radiusSlider->value();
        setSpinBoxValue(m_radius, value);
        emitValueChange(value, value, value, value, EChangedFinished);
    });

    connect(m_left, &CSpinBox::valueChanged, this, [ = ](int value, EChangedPhase phase) {
        Q_UNUSED(value)
        Q_UNUSED(phase)
        emitValueChange(m_left->value(), QVariant(), QVariant(), QVariant());
    });

    connect(m_right, &CSpinBox::valueChanged, this, [ = ](int value, EChangedPhase phase) {
        Q_UNUSED(value)
        Q_UNUSED(phase)
        emitValueChange(QVariant(), m_right->value(), QVariant(), QVariant());
    });

    connect(m_leftBottom, &CSpinBox::valueChanged, this, [ = ](int value, EChangedPhase phase) {
        Q_UNUSED(value)
        Q_UNUSED(phase)
        emitValueChange(QVariant(), QVariant(), m_leftBottom->value(), QVariant());
    });

    connect(m_rightBottom, &CSpinBox::valueChanged, this, [ = ](int value, EChangedPhase phase) {
        Q_UNUSED(value)
        Q_UNUSED(phase)
        emitValueChange(QVariant(), QVariant(), QVariant(), m_rightBottom->value());
    });

    connect(m_radius, &CSpinBox::valueChanged, this, [ = ](int value, EChangedPhase phase) {
        Q_UNUSED(phase)
        QSignalBlocker block(m_radiusSlider);
        m_radiusSlider->setValue(value);
        emitValueChange(value, value, value, value);
    });

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [ = ](DGuiApplicationHelper::ColorType themeType) {
        if (themeType == DGuiApplicationHelper::LightType) {
            m_diffRadiusWidget->setBackgroudColor(QColor(211, 211, 211));
        } else {
            m_diffRadiusWidget->setBackgroudColor(QColor(0, 0, 0, int(0.1 * 255)));
        }
    });
}

void RectRadiusStyleWidget::emitValueChange(QVariant left, QVariant right, QVariant leftBottom, QVariant rightBottom, EChangedPhase phase)
{
    QVariantList l;
    if (m_sameRadiusButton->isChecked()) {
        l << left;
    } else {
        //特殊需求，单个时记录所有修改值，多选只记录修改的角度值
        if (m_drawboard->currentPage()->scene()->selectedItemCount() <= 1) {
            if (!left.isValid()) {
                left = m_left->value();
            }

            if (!right.isValid()) {
                right = m_right->value();
            }

            if (!leftBottom.isValid()) {
                leftBottom = m_leftBottom->value();
            }

            if (!rightBottom.isValid()) {
                rightBottom = m_rightBottom->value();
            }
        }
        l << left << right << leftBottom << rightBottom;
    }

    emit valueChanged(l, phase);
}

void RectRadiusStyleWidget::setSpinBoxValue(CSpinBox *s, int value)
{
    QSignalBlocker block(s);
    s->setValue(value);
}

void RectRadiusStyleWidget::showByChecked(bool bDiffMode)
{
    m_diffRadiusButton->setChecked(bDiffMode);
    m_sameRadiusButton->setChecked(!bDiffMode);

    if (!bDiffMode) {
        m_diffRadiusWidget->hide();
        m_sameRadiusWidget->show();
    } else {
        m_diffRadiusWidget->show();
        m_sameRadiusWidget->hide();
    }
}


void BackgroundWidget::setBackgroudColor(QColor c)
{
    m_bkColor = c;
}

void BackgroundWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.save();
    QPen pen(Qt::transparent, 5);
    painter.setPen(pen);
    QBrush brush(m_bkColor);
    painter.setBrush(brush);
    painter.drawRoundRect(rect(), 10, 10);
    painter.restore();
    QWidget::paintEvent(event);
}
