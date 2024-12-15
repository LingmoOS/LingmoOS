// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "blurattributionwidget.h"
#include <QButtonGroup>
#include "hboxlayoutwidget.h"
#include "boxlayoutwidget.h"
BlurAttributionWidget::BlurAttributionWidget(DrawBoard *drawBoard, QWidget *parent): AttributeWgt(EEBlurStyle, parent)
    , m_drawBoard(drawBoard)
{
    initUi();
}

void BlurAttributionWidget::initUi()
{
    BoxLayoutWidget  *m_blurStyleWidget = new BoxLayoutWidget(this);
    m_blurEffect = new QToolButton(this);
    m_masicoEffect = new QToolButton(this);

    QButtonGroup *button_group = new QButtonGroup(this);
    m_blurEffect->setText(tr("Blur"));
    m_masicoEffect->setText(tr("Mosaic"));

    button_group->addButton(m_blurEffect);
    button_group->addButton(m_masicoEffect);

    m_blurStyleWidget->setMargins(3);

    m_blurEffect->setFixedSize(QSize(125, 30));
    m_masicoEffect->setFixedSize(QSize(125, 30));
    m_blurEffect->setCheckable(true);
    m_masicoEffect->setCheckable(true);
    //设置初始值为高斯模糊
    m_masicoEffect->setChecked(true);

    m_blurStyleWidget->addWidget(m_blurEffect, 1);
    m_blurStyleWidget->addWidget(m_masicoEffect, 2);
    m_blurEffect->setProperty(ChildAttriWidget, true);
    m_masicoEffect->setProperty(ChildAttriWidget, true);

    QVBoxLayout *lay = new QVBoxLayout;
    lay->setMargin(3);
    lay->addWidget(m_blurStyleWidget);
    setLayout(lay);

    connect(button_group, QOverload<QAbstractButton *, bool>::of(&QButtonGroup::buttonToggled), this, [ = ](QAbstractButton * button, bool ischeckd) {
        if (button == m_blurEffect && ischeckd)
            m_drawBoard->setDrawAttribution(EEBlurStyle, BlurEffect);
        else if (button == m_masicoEffect && ischeckd)
            m_drawBoard->setDrawAttribution(EEBlurStyle, MasicoEffect);
    });
}

