// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "blurwidget.h"
#include "widgets/toolbutton.h"
#include "drawshape/cdrawparamsigleton.h"
#include "drawshape/globaldefine.h"
#include "frame/cviewmanagement.h"
#include "frame/cgraphicsview.h"
#include "widgets/cspinbox.h"
#include "application.h"
#include "cattributemanagerwgt.h"

#include <DSlider>
#include <DGuiApplicationHelper>
#include <QButtonGroup>

#include <QHBoxLayout>
#include <QLineEdit>

const int BTN_SPACING = 6;
const int SEPARATE_SPACING = 5;
const int TEXT_SIZE = 14;

const int blur_min_width = 5;
const int blur_max_width = 500;

BlurWidget::BlurWidget(DWidget *parent)
    : DrawAttribution::CAttriBaseOverallWgt(parent)
{
    setAttribution(DrawAttribution::EBlurAttri);
    initUI();
    initConnection();
}

BlurWidget::~BlurWidget()
{
}

void BlurWidget::setBlurWidth(const int &width, bool emitSig)
{
    m_spinboxForLineWidth->blockSignals(true);
    m_spinboxForLineWidth->setValue(width);
    m_spinboxForLineWidth->blockSignals(false);

    if (emitSig) {
        emit blurEffectChanged(getEffect());
    }
}

void BlurWidget::setBlurType(const EBlurEffect &blurEffect, bool emitSig)
{
    auto btn = m_TypeButtons->button(blurEffect);
    if (btn != nullptr) {
        if (!emitSig) {
            m_TypeButtons->blockSignals(true);
        }

        btn->setChecked(true);

        if (!emitSig) {
            m_TypeButtons->blockSignals(false);
        }
    }
}

SBLurEffect BlurWidget::getEffect() const
{
    SBLurEffect effect;
    effect.type = m_TypeButtons->checkedId();
    effect.width = m_spinboxForLineWidth->value();

    return effect;
}

void BlurWidget::initUI()
{
    this->setObjectName("BlurWidget");
    setAttribute(Qt::WA_NoMousePropagation, true);
    DLabel *penLabel = new DLabel(this);
    penLabel->setObjectName("TypeLabel");

    penLabel->setText(tr("Type"));

    QFont ft;
    ft.setPixelSize(TEXT_SIZE);
    penLabel->setFont(ft);

    auto m_blurBtn = new DToolButton(this);
    setWgtAccesibleName(m_blurBtn, "Blur type button");
    m_blurBtn->setMaximumSize(QSize(38, 38));
    m_blurBtn->setIconSize(QSize(38, 38));
    m_blurBtn->setToolTip(tr("Blur"));
    m_blurBtn->setCheckable(true);
    connect(m_blurBtn, &DToolButton::toggled, this, [ = ](bool checked) {
        QIcon icon       = QIcon::fromTheme("ddc_fuzzy tool_normal");
        QIcon activeIcon = QIcon::fromTheme("ddc_fuzzy tool_checked");
        m_blurBtn->setIcon(checked ? activeIcon : icon);
    });
    m_blurBtn->setIcon(QIcon::fromTheme("ddc_fuzzy tool_normal"));

    auto m_masicBtn = new DToolButton(this);
    setWgtAccesibleName(m_masicBtn, "Masic type button");
    m_masicBtn->setMaximumSize(QSize(38, 38));
    m_masicBtn->setIconSize(QSize(38, 38));
    m_masicBtn->setToolTip(tr("Mosaic"));
    m_masicBtn->setCheckable(true);
    connect(m_masicBtn, &DToolButton::toggled, this, [ = ](bool checked) {
        QIcon icon       = QIcon::fromTheme("ddc_smudge tool");
        QIcon activeIcon = QIcon::fromTheme("ddc_smudge tool_checked");
        m_masicBtn->setIcon(checked ? activeIcon : icon);
    });
    m_masicBtn->setIcon(QIcon::fromTheme("ddc_smudge tool"));

    m_TypeButtons = new QButtonGroup(this);
    m_TypeButtons->addButton(m_blurBtn, BlurEffect);
    m_TypeButtons->addButton(m_masicBtn, MasicoEffect);
    m_TypeButtons->setExclusive(true);

    connect(m_TypeButtons, QOverload<int, bool>::of(&QButtonGroup::buttonToggled), this, [ = ](int tp, bool checked) {
        if (checked) {
//            emit blurTypeChanged(EBlurEffect(tp));
            emit blurEffectChanged(getEffect());
        }
    });

    DLabel *penWidthLabel = new DLabel(this);
    penWidthLabel->setObjectName("Width");
    penWidthLabel->setText(tr("Width"));
    penWidthLabel->setFont(ft);

    m_spinboxForLineWidth = new CSpinBox(this);
    m_spinboxForLineWidth->setObjectName("BlurPenWidth");
    m_spinboxForLineWidth->setKeyboardTracking(false);

    m_spinboxForLineWidth->setSpinRange(5, 500);

    m_spinboxForLineWidth->setValue(20);

    if (!Application::isTabletSystemEnvir())
        m_spinboxForLineWidth->setFixedWidth(90);
    m_spinboxForLineWidth->setMaximumHeight(36);
    m_spinboxForLineWidth->setSuffix("px");
    m_spinboxForLineWidth->lineEdit()->setClearButtonEnabled(false);
    m_spinboxForLineWidth->setFont(ft);

    QWidget *groupWidget = new QWidget(this);
    QHBoxLayout *groupLayout = new QHBoxLayout(this);
    groupLayout->addWidget(penWidthLabel);
    groupLayout->addWidget(m_spinboxForLineWidth);
    groupWidget->setLayout(groupLayout);


    m_pLineWidthLabel = new DLabel(this);
    m_pLineWidthLabel->setObjectName("Width Label");
    m_pLineWidthLabel->setText(QString("%1px").arg(m_spinboxForLineWidth->value()));
    m_pLineWidthLabel->setFont(ft);
    m_pLineWidthLabel->setFixedWidth(60);
    m_pLineWidthLabel->hide();

    QHBoxLayout *layout = static_cast<QHBoxLayout *>(centerLayout());

    layout->addWidget(penLabel);
    layout->addWidget(m_blurBtn);
    layout->addWidget(m_masicBtn);
    layout->addWidget(groupWidget);
    setLayout(layout);

    setBlurType(EBlurEffect::MasicoEffect);

    for (int i = 0; i < centerLayout()->count(); ++i) {
        auto item = centerLayout()->itemAt(i);
        if (item->widget() != nullptr) {
            _allWgts.append(item->widget());
        }
    }
}

void BlurWidget::initConnection()
{
    connect(m_spinboxForLineWidth, QOverload<int, EChangedPhase>::of(&CSpinBox::valueChanged), this,
    [ = ](int value, EChangedPhase phase) {
        setBlurWidth(value);
    });
}
