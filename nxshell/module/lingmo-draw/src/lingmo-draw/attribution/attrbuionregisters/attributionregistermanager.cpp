// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "attributionregistermanager.h"
#include "textattributionregister.h"
#include "commonattributionregister.h"
#include "styleattributionregister.h"
#include "styleattriwidget.h"

void AttributionRegisterManager::registe()
{
    addDefaultAttri();

    for (IAttributionRegister *r : m_registers) {
        r->registe();
    }
}

void AttributionRegisterManager::addRegister(IAttributionRegister *pRegister)
{
    pRegister->setDrawBoard(drawBoard());
    m_registers.append(pRegister);
}

AttributionRegisterManager::~AttributionRegisterManager()
{
    for (int i = m_registers.count() - 1; i >= 0; i--) {
        delete m_registers.takeAt(i);
    }
}

void AttributionRegisterManager::addDefaultAttri()
{
    CommonAttributionRegister *commonAttr = new CommonAttributionRegister(drawBoard());
    StyleAttributionRegister *styleAtti = new StyleAttributionRegister(drawBoard());

    commonAttr->registe();

    auto ls = commonAttr->getStyleAttriWidgets();
    for (QWidget *l : ls) {
        styleAtti->getStyleWidget()->addChildAtrri(dynamic_cast<AttributeWgt *>(l));
    }
    styleAtti->registe();

}
