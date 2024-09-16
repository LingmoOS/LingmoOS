// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <DArrowLineExpand>
#include <DWidget>
#include <DSplitter>

#include "styleattriwidget.h"

StyleAttriWidget::StyleAttriWidget(int attri, QWidget *parent): AttributeWgt(attri, parent)
{
    initUi();
}

QVBoxLayout *StyleAttriWidget::getLayout()
{
    return m_layout;//dynamic_cast<QVBoxLayout *>(layout());
}

void StyleAttriWidget::removeChildAtrri(AttributeWgt *attri)
{
    m_childAtrri.removeOne(attri);
}

void StyleAttriWidget::addChildAtrri(AttributeWgt *attri, bool bAddLayout)
{
    attri->setProperty(ChildAttriWidget, true);
    attri->hide();
    m_childAtrri.append(attri);

    attri->setParent(this);
    if (bAddLayout) {
        m_layout->addWidget(attri);
    }

}

void StyleAttriWidget::setVar(const QVariant &var)
{
}

void StyleAttriWidget::initUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setMargin(0);
    this->setLayout(mainLayout);

    m_layout = new QVBoxLayout;
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setMargin(0);

    auto l =  new QLabel(tr("Style"));
    QFont temp("SourceHanSansSc", 14);
    l->setFont(temp);
    mainLayout->addWidget(l);
    addHSeparateLine();

    mainLayout->addLayout(m_layout);
}
