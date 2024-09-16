// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "morepicfloatwidget.h"
#include "accessibility/ac-desktop-define.h"

#include <QColor>

#include <DGuiApplicationHelper>

MorePicFloatWidget::MorePicFloatWidget(QWidget *parent)
    : DFloatingWidget(parent)
{

}

MorePicFloatWidget::~MorePicFloatWidget()
{

}

void MorePicFloatWidget::initUI()
{
    setBlurBackgroundEnabled(true);
    m_pLayout = new QVBoxLayout(this);
    this->setLayout(m_pLayout);
    m_buttonUp = new  DIconButton(this);
    m_buttonDown = new  DIconButton(this);
    m_labelNum = new DLabel(this);
    m_pLayout->addWidget(m_labelNum);
    m_labelNum->setAlignment(Qt::AlignCenter);
    m_labelNum->setText("0/0");

    m_buttonUp->setIcon(QIcon::fromTheme("dcc_up"));
    m_buttonUp->setIconSize(QSize(40, 40));
    m_buttonUp->setObjectName(MOREPIC_UP_BUTTON);
    m_buttonUp->setFixedSize(QSize(42, 42));

    m_buttonDown->setIcon(QIcon::fromTheme("dcc_down"));
    m_buttonDown->setIconSize(QSize(40, 40));
    m_buttonDown->setObjectName(MOREPIC_DOWN_BUTTON);
    m_buttonDown->setFixedSize(QSize(42, 42));

    DPalette pa1 = m_buttonUp->palette();
    DPalette pa2 = m_buttonDown->palette();;
    if (DGuiApplicationHelper::LightType == DGuiApplicationHelper::instance()->themeType()) {
        pa1.setColor(DPalette::Light, QColor(255, 255, 255, 255));
        pa1.setColor(DPalette::Dark, QColor(255, 255, 255, 255));

        pa2.setColor(DPalette::Light, QColor(255, 255, 255, 255));
        pa2.setColor(DPalette::Dark, QColor(255, 255, 255, 255));
    } else {
        pa1.setColor(DPalette::Light, QColor(40, 40, 40, 255));
        pa1.setColor(DPalette::Dark, QColor(40, 40, 40, 255));

        pa2.setColor(DPalette::Light, QColor(40, 40, 40, 255));
        pa2.setColor(DPalette::Dark, QColor(40, 40, 40, 255));
    }
    m_buttonUp->setPalette(pa1);
    m_buttonDown->setPalette(pa2);

    m_pLayout->addWidget(m_buttonUp);
    m_pLayout->addWidget(m_buttonDown);

    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
    this, [ = ]() {
        if (!m_buttonUp && !m_buttonDown) {
            return;
        }
        DGuiApplicationHelper::ColorType themeType =
            DGuiApplicationHelper::instance()->themeType();
        DPalette pa1 = m_buttonUp->palette();
        DPalette pa2 = m_buttonDown->palette();
        if (themeType == DGuiApplicationHelper::LightType) {
            pa1.setColor(DPalette::Light, QColor(255, 255, 255, 255));
            pa2.setColor(DPalette::Light, QColor(255, 255, 255, 255));

            pa1.setColor(DPalette::Dark, QColor(255, 255, 255, 255));
            pa2.setColor(DPalette::Dark, QColor(255, 255, 255, 255));
        } else {
            pa1.setColor(DPalette::Light, QColor(40, 40, 40, 255));
            pa2.setColor(DPalette::Light, QColor(40, 40, 40, 255));

            pa1.setColor(DPalette::Dark, QColor(40, 40, 40, 255));
            pa2.setColor(DPalette::Dark, QColor(40, 40, 40, 255));
        }
        m_buttonUp->setPalette(pa1);
        m_buttonDown->setPalette(pa2);
    });


}

DIconButton *MorePicFloatWidget::getButtonUp()
{
    return m_buttonUp;
}

DIconButton *MorePicFloatWidget::getButtonDown()
{
    return m_buttonDown;
}

void MorePicFloatWidget::setLabelText(const QString &num)
{
    m_labelNum->setText(num);
}
