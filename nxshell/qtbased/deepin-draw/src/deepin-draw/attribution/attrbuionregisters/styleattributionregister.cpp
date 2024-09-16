// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "styleattributionregister.h"
#include "styleattriwidget.h"
#include "commonattributionregister.h"
#include "sliderspinboxwidget.h"
#include "hboxlayoutwidget.h"
#include "itemstyleregisters.h"
#include "colorstylewidget.h"
#include "rectradiusstylewidget.h"
#include "textattributionregister.h"

StyleAttributionRegister::StyleAttributionRegister(DrawBoard *drawboard): IAttributionRegister(drawboard)
{
    m_styleAttri = new StyleAttriWidget(EStyleProper, drawboard);
    m_styleAttri->setProperty(ParentAttriWidget, true);
}

void StyleAttributionRegister::registe()
{
    drawBoard()->attributionManager()->installComAttributeWgt(m_styleAttri->attribution(), m_styleAttri, QVariant());
    registeStyles();
}

StyleAttriWidget *StyleAttributionRegister::getStyleWidget()
{
    return m_styleAttri;
}

void StyleAttributionRegister::registeStyles()
{
    //自适应属性
    IAttributionRegister *rg = new AdjustmentAttriRegister(drawBoard(), this);
    rg->registe();

    //锚点属性
    rg = new StarAnchorAttriRegister(drawBoard(), this);
    rg->registe();
    m_styleAttri->addChildAtrri(dynamic_cast<StarAnchorAttriRegister *>(rg)->attriWidget());

    //星型属性
    rg = new StartAttriRegister(drawBoard(), this);
    rg->registe();
    m_styleAttri->addChildAtrri(dynamic_cast<StartAttriRegister *>(rg)->attriWidget());

    //多边形属性
    rg = new PolygonSidesAttriRegister(drawBoard(), this);
    rg->registe();
    m_styleAttri->addChildAtrri(dynamic_cast<PolygonSidesAttriRegister *>(rg)->attriWidget());

    //直线属性
    rg = new LineAttriRegister(drawBoard(), this);
    rg->registe();
    m_styleAttri->addChildAtrri(dynamic_cast<LineAttriRegister *>(rg)->attriWidget());

    //橡皮擦属性
    rg = new EraserAttriRegister(drawBoard(), this);
    rg->registe();

    //模糊属性
    rg = new BlurAttriRegister(drawBoard(), this);
    rg->registe();

    //分割属性
    rg = new CutAttriRegister(drawBoard(), this);
    rg->registe();

    //画笔属性
    rg = new PenAttriRegister(drawBoard(), this);
    rg->registe();
    m_styleAttri->addChildAtrri(dynamic_cast<PenAttriRegister *>(rg)->attriWidget());
    m_styleAttri->addChildAtrri(dynamic_cast<PenAttriRegister *>(rg)->penStyleSettingWgt());

    //矩形属性
    rg = new RectAttriRegister(drawBoard(), this);
    rg->registe();
    m_styleAttri->addChildAtrri(dynamic_cast<RectAttriRegister *>(rg)->attriWidget());

    //文本属性
    rg = new TextAttributionRegister(drawBoard(), this);
    rg->registe();

    auto ls = dynamic_cast<TextAttributionRegister *>(rg)->getAttriWidgets();
    for (QWidget *l : ls) {
        m_styleAttri->addChildAtrri(dynamic_cast<AttributeWgt *>(l));
    }
}


