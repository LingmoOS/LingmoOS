// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cmasicotool.h"
//#include "cgraphicsmasicoitem.h"
#include "cdrawscene.h"
#include "cdrawparamsigleton.h"
#include "cdrawtoolmanagersigleton.h"
#include "frame/cviewmanagement.h"
#include "frame/cgraphicsview.h"
#include "cgraphicsitemselectedmgr.h"
#include "cpictureitem.h"
#include "global.h"
#include "application.h"
#include "cundoredocommand.h"
#include "cattributeitemwidget.h"
#include "cattributemanagerwgt.h"
#include "blurwidget.h"
#include "cdrawscene.h"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QApplication>
#include <DToolButton>
#include <QDebug>

IBlurTool::IBlurTool(QObject *parent): IDrawTool(blur)
{
    setParent(parent);

    connect(this, &IBlurTool::boardChanged, this, [ = ](DrawBoard * old, DrawBoard * cur) {
        Q_UNUSED(old)
        for (int i = 0; i < cur->count(); ++i) {
            auto page = cur->page(i);
            connect(page->scene(), &PageScene::selectionChanged, this, [ = ](const QList<CGraphicsItem * > &children) {
                Q_UNUSED(children)
                setEnable(isEnable(page->view()));
            });
        }
        connect(cur, &DrawBoard::pageAdded, this, [ = ](Page * added) {
            setEnable(isEnable(added->view()));
            connect(added->scene(), &PageScene::selectionChanged, this, [ = ](const QList<CGraphicsItem * > &children) {
                Q_UNUSED(children)
                setEnable(isEnable(added->view()));
            });
        });
        connect(cur, QOverload<Page *>::of(&DrawBoard::currentPageChanged), this, [ = ](Page * cur) {
            setEnable(isEnable(cur->view()));
        });
    });
}

SAttrisList IBlurTool::attributions()
{
    DrawAttribution::SAttrisList result;
    result << defaultAttriVar(DrawAttribution::EBlurAttri);
    //result << defaultAttriVar(DrawAttribution::BlurPenWidth);
    //result << defaultAttriVar(DrawAttribution::BlurPenEffect);
    return result;
}

QCursor IBlurTool::cursor() const
{
    static QPixmap s_cur = QPixmap(":/cursorIcons/smudge_mouse.svg");
    return QCursor(s_cur);
}

bool IBlurTool::isBlurEnable(const CGraphicsItem *pItem)
{
    return _blurEnableItems.contains(const_cast<CGraphicsItem *>(pItem));
}

QList<CGraphicsItem *> IBlurTool::getBlurEnableItems(const CGraphicsItem *pItem)
{
    QList<CGraphicsItem *> resultItems;
    if (pItem == nullptr)
        return QList<CGraphicsItem *>();

    if (pItem->isBzGroup()) {
        auto items = static_cast<const CGraphicsItemGroup *>(pItem)->items(true);
        for (auto p : items) {
            if (p->isBlurEnable()) {
                resultItems.append(const_cast<CGraphicsItem *>(p));
            }
        }
    } else {
        if (pItem->isBlurEnable()) {
            resultItems.append(const_cast<CGraphicsItem *>(pItem));
        }
    }
    return resultItems;
}

void IBlurTool::saveZ(PageScene *scene)
{
    _tempZs.clear();
    auto invokedItems = scene->getRootItems(PageScene::EAesSort);
    for (int i = 0; i < invokedItems.size(); ++i) {
        auto pItem = invokedItems.at(i);
        saveItemZValue(pItem);
    }
}

void IBlurTool::restoreZ()
{
    for (auto it = _tempZs.begin(); it != _tempZs.end(); ++it) {
        it.key()->setZValue(it.value());

        auto pLayer = dynamic_cast<JDynamicLayer *>(it.key());
        if (nullptr !=  pLayer) {
            auto p = new JGeomeCommand(pLayer);
            pLayer->appendComand(p, false, false);
        }
    }
    _tempZs.clear();
}

void IBlurTool::saveItemZValue(CGraphicsItem *pItem)
{
    if (pItem->isBzItem()) {
        _tempZs.insert(pItem, pItem->drawZValue());
    } else if (pItem->isBzGroup()) {
        auto items = static_cast<CGraphicsItemGroup *>(pItem)->items();
        for (auto p : items) {
            saveItemZValue(p);
            _tempZs.insert(pItem, pItem->drawZValue());
        }
    }
}

QAbstractButton *IBlurTool::initToolButton()
{
    DToolButton *m_blurBtn = new DToolButton;
    m_blurBtn->setShortcut(QKeySequence(QKeySequence(Qt::Key_B)));
    setWgtAccesibleName(m_blurBtn, "Blur tool button");
    m_blurBtn->setToolTip(tr("Blur (B)"));
    m_blurBtn->setIconSize(QSize(48, 48));
    m_blurBtn->setFixedSize(QSize(37, 37));
    m_blurBtn->setCheckable(true);
    m_blurBtn->setEnabled(false);

    connect(m_blurBtn, &DToolButton::toggled, m_blurBtn, [ = ](bool b) {
        QIcon icon       = QIcon::fromTheme("ddc_smudge tool_normal");
        QIcon activeIcon = QIcon::fromTheme("ddc_smudge tool_active");
        m_blurBtn->setIcon(b ? activeIcon : icon);
    });
    m_blurBtn->setIcon(QIcon::fromTheme("ddc_smudge tool_normal"));

    return m_blurBtn;
}

void IBlurTool::registerAttributionWidgets()
{
    auto pBlurWidget = new BlurWidget;
//    connect(pBlurWidget, &BlurWidget::blurWidthChanged, this, [ = ](int width) {

//        //drawBoard()->setDrawAttribution(DrawAttribution::BlurPenWidth, width, EChanged, false);

//        QVariant var1; var1.setValue<int>(width);
//        QVariant var2; var2.setValue<int>(int(MasicoEffect));
//        QVariant var; var.setValue<QList<QVariant>>(QList<QVariant>() << var1 << var2);
//        drawBoard()->setDrawAttribution(DrawAttribution::EBlurAttri, var, EChanged, false);
//    });

//    connect(pBlurWidget, &BlurWidget::blurTypeChanged, this, [ = ](EBlurEffect type) {
//        //drawBoard()->setDrawAttribution(DrawAttribution::BlurPenEffect, int(type), EChanged, false);

//        QVariant var1; var1.setValue<int>(20);
//        QVariant var2; var2.setValue<int>(int(type));
//        QVariant var; var.setValue<QList<QVariant>>(QList<QVariant>() << var1 << var2);
//        drawBoard()->setDrawAttribution(DrawAttribution::EBlurAttri, var, EChanged, false);
//    });
    connect(pBlurWidget, &BlurWidget::blurEffectChanged, this, [ = ](const SBLurEffect & effect) {

        //drawBoard()->setDrawAttribution(DrawAttribution::BlurPenWidth, width, EChanged, false);

        QVariant var;
        var.setValue<SBLurEffect>(effect);
        drawBoard()->setDrawAttribution(DrawAttribution::EBlurAttri, var, EChanged, false);
    });

    connect(drawBoard()->attributionWidget(), &CAttributeManagerWgt::updateWgt, pBlurWidget,
    [ = ](QWidget * pWgt, const QVariant & var) {
        if (pWgt == pBlurWidget) {
            QSignalBlocker bloker(pBlurWidget);

            int width = var.value<SBLurEffect>().width;
            int type = var.value<SBLurEffect>().type;

            pBlurWidget->setBlurType(EBlurEffect(type));
            pBlurWidget->setBlurWidth(width);
        }
    });

    //drawBoard()->attributionWidget()->setDefaultAttributionVar(DrawAttribution::BlurPenWidth, 20);
    //drawBoard()->attributionWidget()->setDefaultAttributionVar(DrawAttribution::BlurPenEffect, int(MasicoEffect));
    QVariant var;
    var.setValue<SBLurEffect>(SBLurEffect());
    drawBoard()->attributionWidget()->installComAttributeWgt(DrawAttribution::EBlurAttri, pBlurWidget, var);
}

void IBlurTool::toolStart(CDrawToolEvent *event, ITERecordInfo *pInfo)
{
    if (isFirstEvent()) {
        //获取到这次能进行模糊的图元
        auto selectItems      = event->scene()->selectGroup()->items();
        auto pCurSelectItem   = selectItems.count() == 1 ? selectItems.first() : nullptr;

        _blurEnableItems      = getBlurEnableItems(pCurSelectItem);

        if (_blurEnableItems.count() > 0) {
            _pressedPosBlurEnable = true;

            //置顶前,收集到可能会被模糊的图片图元数据(置顶后再获取到图元z值的话就不是最原始的z值了)
            event->scene()->recordItemsInfoToCmd(_blurEnableItems,
                                                 UndoVar, true);

            //置顶功能:实现进行模糊时,被模糊的图元置顶
            //置顶功能Step1.保存场景当前图元的z值
            saveZ(event->scene());

            //置顶功能Step2.置顶
            event->scene()->moveBzItemsLayer(selectItems, EUpLayer, -1);

        } else {
            _pressedPosBlurEnable = false;
        }

        //点下时进行鼠标样式的初始化:鼠标样式是否设置为模糊样式的条件
        bool cursorBlurEnable = (pCurSelectItem != nullptr) && (pCurSelectItem->sceneBoundingRect().contains(event->pos()));
        this->drawBoard()->currentPage()->setDrawCursor(cursorBlurEnable ? cursor() : QCursor(Qt::ForbiddenCursor));
    }
    //点击绘制
    toolDoUpdate(event);
}

int IBlurTool::decideUpdate(CDrawToolEvent *event, ITERecordInfo *pInfo)
{
    Q_UNUSED(event)
    Q_UNUSED(pInfo)

    if (_pressedPosBlurEnable) {
        enum {EDoBLur = 1};
        return EDoBLur;
    }
    return 0;
}

void IBlurTool::toolUpdate(CDrawToolEvent *event, ITERecordInfo *pInfo)
{
    if (!_pressedPosBlurEnable)
        return;

    //0.获取到当前最上层的图元是
    auto currenItem = event->scene()->topBzItem(event->pos(), true,
                                                event->eventType() == CDrawToolEvent::ETouchEvent ? drawBoard()->touchFeelingEnhanceValue() : 0);

    bool blurActive = isBlurEnable(currenItem);

    //1.顶层图元发生改变或没变化的情况处理
    if (_pLastTopItem != currenItem) {
        //1.1:那么原来的顶层图元_pLastTopItem如果可以模糊那么需要进行模糊路径的结束
        if (isBlurEnable(_pLastTopItem)) {
            _pLastTopItem->blurEnd();
        }
        //1.2:新的顶层图元currenItem如果可以模糊那么进行模糊开始
        if (blurActive) {
            currenItem->blurBegin(currenItem->mapFromScene(event->pos()));
            currenItem->blurUpdate(currenItem->mapFromScene(event->pos()), true);
            _bluringItemsSet.insert(currenItem);
        }
        _pLastTopItem = currenItem;
    } else {
        //1.3:顶层图元未发生改变:那么直接进行模糊路径刷新即可
        if (blurActive) {
            currenItem->blurUpdate(currenItem->mapFromScene(event->pos()), true);
        }
    }

    //2.更新鼠标样式
    this->drawBoard()->currentPage()->setDrawCursor(blurActive ? cursor() : QCursor(Qt::ForbiddenCursor));
}

void IBlurTool::toolFinish(CDrawToolEvent *event, ITERecordInfo *pInfo)
{
    if (isFinalEvent()) {
        if (_pressedPosBlurEnable) {

            //置顶功能Step3.还原z值
            restoreZ();

            _pressedPosBlurEnable = false;

            enum {EDoBLur = 1};
            if (pInfo->_opeTpUpdate == EDoBLur) {
                for (auto pItem : _bluringItemsSet) {

                    if (pItem->isBlurActived()) {
                        pItem->blurEnd();
                    }

                    event->scene()->recordItemsInfoToCmd(QList<CGraphicsItem *>() << pItem,
                                                         RedoVar, false);
                }
                event->scene()->finishRecord(false);

                _bluringItemsSet.clear();
            } else {
                //没有移动那么就直接清理命令
                CUndoRedoCommand::clearCommand();
            }
        }

        _pLastTopItem = nullptr;

        //恢复光标
        this->drawBoard()->currentPage()->setDrawCursor(cursor());
    }
}

bool IBlurTool::returnToSelectTool(CDrawToolEvent *event, ITERecordInfo *pInfo)
{
    return false;
}

bool IBlurTool::isEnable(PageView *pView)
{
    if (pView == nullptr)
        return false;

    auto items = pView->drawScene()->selectGroup()->items();
    bool isBlur = false;
    if (items.count() == 1) {
        CGraphicsItem *pItem = items[0];

        if (pItem->isBzGroup()) {
            QList<CGraphicsItem *> lists = static_cast<CGraphicsItemGroup *>(pItem)->getBzItems(true);
            foreach (CGraphicsItem *p, lists) {
                if (p->isBlurEnable()) {
                    isBlur = true;
                    break;
                }
            }

        } else {
            if (pItem->isBlurEnable())
                isBlur = true;
        }
    }
    return isBlur;
}

int IBlurTool::minMoveUpdateDistance()
{
    return 0;
}

int IBlurTool::allowedMaxTouchPointCount()
{
    return 1;
}

void IBlurTool::onStatusChanged(EStatus oldStatus, EStatus nowStatus)
{
    Q_UNUSED(oldStatus)
    Q_UNUSED(nowStatus)

    if (oldStatus == EIdle && nowStatus == EReady) {
        qApp->installEventFilter(this);
    } else if (oldStatus == EReady && nowStatus == EIdle) {
        qApp->removeEventFilter(this);
    }
}

bool IBlurTool::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::ShortcutOverride) {
        bool b = IDrawTool::eventFilter(o, e);
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(e);
        if (keyEvent->matches(QKeySequence::Redo) || keyEvent->matches(QKeySequence::Undo)) {
            QMetaObject::invokeMethod(this, [ = ]() {
                if (drawBoard()->currentPage() != nullptr) {
                    auto scene = drawBoard()->currentPage()->scene();
                    if (!isEnable(drawBoard()->currentPage()->view())) {
                        drawBoard()->setCurrentTool(selection);
                        scene->drawView()->viewport()->setCursor(Qt::ArrowCursor);
                    }
                }
            }, Qt::QueuedConnection);
        }
        return b;
    }
    return IDrawTool::eventFilter(o, e);
}
