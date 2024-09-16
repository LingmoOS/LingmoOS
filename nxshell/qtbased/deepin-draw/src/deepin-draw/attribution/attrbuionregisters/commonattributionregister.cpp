// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QButtonGroup>
#include <QAbstractItemView>
#include <QPoint>
#include <QObject>
#include <QListView>
#include <QFontComboBox>

#include "commonattributionregister.h"
#include "pageitem.h"
#include "drawboard.h"
#include "pagescene.h"
#include "pageview.h"
#include "cundoredocommand.h"
#include "colorstylewidget.h"
#include "rectradiusstylewidget.h"
#include "cspinbox.h"
#include "sliderspinboxwidget.h"
#include "attributewidget.h"
#include "application.h"
#include "hboxlayoutwidget.h"
#include "groupbuttonwidget.h"
#include "rotateattriwidget.h"
#include "orderwidget.h"
#include "globaldefine.h"
#include "drawtool.h"

/**
 * @brief 注册各项属性工具栏控件，注册仅调用一次
 */
void CommonAttributionRegister::registe()
{
    if (m_isInit) {
        return;
    }
    m_isInit = true;

    //注册基础样式属性
    registeBaseStyleAttrri();
    //注册组合属性
    registeGroupAttri();
    //注册顺序属性
    registeOrderAttri();
    //注册选择属性
    resgisteRotateAttri();
}


/**
 * @brief 注册属性工具栏基础样式工具，包括颜色，画笔宽度等
 */
void CommonAttributionRegister::registeBaseStyleAttrri()
{
    if (nullptr != m_fillBrushStyle
            || nullptr != m_borderPenStyle
            || nullptr != m_penWidth) {
        return;
    }

    m_fillBrushStyle = new ColorStyleWidget(drawBoard());
    m_fillBrushStyle->setColorFill(0);
    m_borderPenStyle = new ColorStyleWidget(drawBoard());
    m_borderPenStyle->setTitleText(tr("Stroke"));
    m_borderPenStyle->setColorFill(0);
    m_borderPenStyle->setColorTextVisible(false);
    drawBoard()->attributionManager()->installComAttributeWgt(EBrushColor, m_fillBrushStyle, QColor(0, 0, 0));
    drawBoard()->attributionManager()->installComAttributeWgt(EPenColor, m_borderPenStyle, QColor(0, 0, 0));

    connect(m_fillBrushStyle, &ColorStyleWidget::colorChanged, this, [ = ](const QColor & color, int phase) {

        drawBoard()->setDrawAttribution(EBrushColor, color, phase);
    });

    connect(m_borderPenStyle, &ColorStyleWidget::colorChanged, this, [ = ](const QColor & color, int phase) {
        drawBoard()->setDrawAttribution(EPenColor, color, phase);
    });

    m_penWidth = new CSpinBox(drawBoard());
    m_penWidth->setEnabledEmbedStyle(true);
    m_penWidth->setSpinRange(0, 10);
    m_penWidth->setMaximumWidth(103);
    m_penWidth->setProperty(ChildAttriWidget, true);


    QObject::connect(m_penWidth, &CSpinBox::valueChanged, m_penWidth, [ = ](int value, EChangedPhase phase) {
        Q_UNUSED(phase)
        drawBoard()->setDrawAttribution(EPenWidth, value);
    });

    connect(drawBoard()->attributionManager()->helper(), &AttributionManagerHelper::updateWgt, m_penWidth,
    [ = ](QWidget * pWgt, const QVariant & var) {
        if (pWgt == m_penWidth) {
            QSignalBlocker bloker(m_penWidth);
            if (!var.isValid()) {
                m_penWidth->setSpecialText(SPECIAL_TEXT);
            } else {
                m_penWidth->setValue(var.toInt());
            }
        }
    });
    drawBoard()->attributionManager()->installComAttributeWgt(EPenWidth, m_penWidth, 2);

    //在border中显示
    m_borderPenStyle->addWidget(m_penWidth, 2);

    m_enablePenStyle = new CheckBoxSettingWgt("", m_borderPenStyle);
    m_enablePenStyle->setAttribution(EEnablePenStyle);
    drawBoard()->attributionManager()->installComAttributeWgt(m_enablePenStyle->attribution(), m_enablePenStyle, true);
    QObject::connect(m_enablePenStyle, &CheckBoxSettingWgt::checkChanged, drawBoard(), [ = ](bool value) {
        drawBoard()->setDrawAttribution(m_enablePenStyle->attribution(), value);
        m_borderPenStyle->setContentEnable(value);
    });

    QObject::connect(m_enablePenStyle, &CheckBoxSettingWgt::checkEnable, drawBoard(), [ = ](bool value) {
        m_borderPenStyle->setContentEnable(value);
    });

    QObject::connect(m_enablePenStyle, &CheckBoxSettingWgt::checkStatusChanged, drawBoard(), [ = ](int status) {
        m_borderPenStyle->setContentEnable(status != Qt::CheckState::Unchecked);
    });

    QObject::connect(m_borderPenStyle, &ColorStyleWidget::colorChanged, drawBoard(), [ = ] {
        if (m_enablePenStyle->checkBox()->isTristate())
        {
            emit m_enablePenStyle->checkChanged(true);
            m_enablePenStyle->checkBox()->setTristate(false);
            m_enablePenStyle->checkBox()->setChecked(true);
            m_enablePenStyle->checkBox()->update();
        }
    });

    m_enableBrushStyle = new CheckBoxSettingWgt("", m_fillBrushStyle);
    m_enableBrushStyle->setAttribution(EEnableBrushStyle);
    drawBoard()->attributionManager()->installComAttributeWgt(m_enableBrushStyle->attribution(), m_enableBrushStyle, true);
    QObject::connect(m_enableBrushStyle, &CheckBoxSettingWgt::checkChanged, drawBoard(), [ = ](bool value) {
        m_fillBrushStyle->setContentEnable(value);
        drawBoard()->setDrawAttribution(m_enableBrushStyle->attribution(), value);
    });
    QObject::connect(m_enableBrushStyle, &CheckBoxSettingWgt::checkEnable, drawBoard(), [ = ](bool value) {
        m_fillBrushStyle->setContentEnable(value);
    });
    QObject::connect(m_enableBrushStyle, &CheckBoxSettingWgt::checkStatusChanged, drawBoard(), [ = ](int status) {
        m_fillBrushStyle->setContentEnable(status != Qt::CheckState::Unchecked);
    });

    QObject::connect(m_fillBrushStyle, &ColorStyleWidget::colorChanged, drawBoard(), [ = ] {
        if (m_enableBrushStyle->checkBox()->isTristate())
        {
            emit m_enableBrushStyle->checkChanged(true);
            m_enableBrushStyle->checkBox()->setTristate(false);
            m_enableBrushStyle->checkBox()->setChecked(true);
            m_enableBrushStyle->checkBox()->update();
        }
    });


    m_borderPenStyle->setProperty(ChildAttriWidget, true);
    m_fillBrushStyle->setProperty(ChildAttriWidget, true);
    m_borderPenStyle->addTitleWidget(m_enablePenStyle, Qt::AlignRight);
    m_fillBrushStyle->addTitleWidget(m_enableBrushStyle, Qt::AlignRight);
}

void CommonAttributionRegister::registeOrderAttri()
{
    OrderAttriRegister *rg = new OrderAttriRegister(drawBoard(), this);
    rg->registe();
}

void CommonAttributionRegister::resgisteRotateAttri()
{
    RotateAttriRegister *rg = new RotateAttriRegister(drawBoard(), this);
    rg->registe();
}

QList<QWidget *> CommonAttributionRegister::getStyleAttriWidgets()
{
    QList<QWidget *> l;
    l.append(m_fillBrushStyle);
    l.append(m_borderPenStyle);

    return l;
}

void CommonAttributionRegister::registeGroupAttri()
{
    GroupAttriRegister *rg = new GroupAttriRegister(drawBoard(), this);
    rg->registe();
}

void GroupAttriRegister::registe()
{
    if (nullptr != m_groupWidget) {
        return;
    }

    m_groupWidget = new GroupButtonWidget(drawBoard());
    setWgtAccesibleName(m_groupWidget, "groupButtonWidget");
    drawBoard()->attributionManager()->installComAttributeWgt(EGroupWgt, m_groupWidget);

    connect(m_groupWidget, &GroupButtonWidget::buttonClicked, this, [ = ](bool doGroup, bool doUngroup) {
        auto currentPage = drawBoard()->currentPage();
        if (currentPage == nullptr)
            return;

        auto currentScene = currentPage->scene();
        if (doGroup) {
            auto tmpGroupItem = currentScene->creatGroup(currentScene->selectedPageItems(), nullptr);
            currentScene->clearSelections();
            currentScene->selectPageItem(tmpGroupItem);
            updateGroupStatus();

            if (tmpGroupItem != nullptr) {
                UndoRecorder recorder(currentScene->currentTopLayer(), LayerUndoCommand::ChildGroupAdded,
                                      QList<PageItem *>() << tmpGroupItem << currentScene->selectedPageItems());
                currentScene->clearSelections();
                currentScene->selectPageItem(tmpGroupItem);
            }
        }

        if (doUngroup) {
            auto selectedItems = currentScene->selectedPageItems();
            if (!selectedItems.isEmpty()) {
                for (auto item : selectedItems) {
                    if (item->type() == GroupItemType) {
                        auto gp = static_cast<GroupItem *>(item);
                        UndoRecorder recorder(currentScene->currentTopLayer(), LayerUndoCommand::ChildGroupRemoved,
                                              QList<PageItem *>() << gp << gp->items());
                        currentScene->cancelGroup(gp);
                    }
                }
            }
        }
        //另外需要将焦点转移到text
        auto pView = drawBoard()->currentPage()->view();
        pView->setFocus();
    });

    connect(drawBoard()->attributionManager()->helper(), &AttributionManagerHelper::updateWgt, this,
    [ = ](QWidget * pWgt, const QVariant & var) {
        Q_UNUSED(var)
        if (pWgt == m_groupWidget) {
            updateGroupStatus();
        }
    });
}

void GroupAttriRegister::updateGroupStatus()
{
    PageScene *curScene = drawBoard()->currentPage()->scene();
    if (nullptr == curScene) {
        return;
    }

    QList<QVariant> couple;
    bool canGroup = curScene->isGroupable(curScene->selectedPageItems());
    bool canUnGroup = curScene->isUnGroupable(curScene->selectedPageItems());

    m_groupWidget->setGroupFlag(canGroup, canUnGroup);
}

void RotateAttriRegister::registe()
{
    if (nullptr != m_rotateAttri) {
        return;
    }

    m_rotateAttri = new RotateAttriWidget(drawBoard());
    setWgtAccesibleName(m_rotateAttri, "rotateAttriWidget");
    drawBoard()->attributionManager()->installComAttributeWgt(ERotProperty, m_rotateAttri, 0);
    // 关联旋转属性控件控制的角度变更信号，当图元角度变更时，更新属性值
    connect(drawBoard()->attributionManager()->helper(), &AttributionManagerHelper::internalAttibutionUpdate, this,
    [ = ](int attris, const QVariant & var, int) {
        if (ERotProperty == attris) {
            m_rotateAttri->setVar(var);
        }
    });

    connect(drawBoard()->tool(pen), &DrawTool::statusChanged, this,
    [ = ](DrawTool::EStatus oldStatus, DrawTool::EStatus nowStatus) {
        if (oldStatus == DrawTool::EReady && nowStatus == DrawTool::EWorking)
            m_rotateAttri->setEnabled(false);
        else
            m_rotateAttri->setEnabled(true);
    });
}

void OrderAttriRegister::registe()
{
    if (nullptr != m_orderAttri) {
        return;
    }

    m_orderAttri = new OrderWidget(drawBoard());
    setWgtAccesibleName(m_orderAttri, "orderWidget");
    drawBoard()->attributionManager()->installComAttributeWgt(EOrderProperty, m_orderAttri, 0);
    connect(drawBoard()->attributionManager()->helper(), &AttributionManagerHelper::internalAttibutionUpdate, this,
    [ = ](int attris, const QVariant & var, int) {
        Q_UNUSED(var)
        if (EOrderProperty == attris) {
            m_orderAttri->updateLayerButtonStatus();
        }
    });

    connect(drawBoard(), qOverload<Page *>(&DrawBoard::currentPageChanged), this, [ = ](Page * page) {
        if (page->scene()->selectedItemCount() <= 0) {
            m_orderAttri->setEnabled(false);
        }
    });
    connect(drawBoard()->tool(pen), &DrawTool::statusChanged, this,
    [ = ](DrawTool::EStatus oldStatus, DrawTool::EStatus nowStatus) {
        if (oldStatus == DrawTool::EReady && nowStatus == DrawTool::EWorking)
            m_orderAttri->setEnabled(false);
        else
            m_orderAttri->setEnabled(true);
    });
}
