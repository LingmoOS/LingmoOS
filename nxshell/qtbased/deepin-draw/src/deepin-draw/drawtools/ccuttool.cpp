// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ccuttool.h"
//#include "cdrawscene.h"
//#include "cdrawparamsigleton.h"
#include "qgraphicsview.h"
#include "globaldefine.h"

#include <DApplication>

#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <DToolButton>
#include <QAbstractButton>
#include "layeritem.h"
#include "application.h"
//#include "frame/cviewmanagement.h"
//#include "frame/cgraphicsview.h"

//#include "drawshape/globaldefine.h"
//#include "drawshape/cdrawparamsigleton.h"

#include "application.h"
#include "toptoolbar.h"
//#include "ccutwidget.h"
//#include "cattributeitemwidget.h"
//#include "ccutwidget.h"
//#include "cattributemanagerwgt.h"
#include "cundoredocommand.h"
#include "ccutdialog.h"
#include "pageview.h"

DWIDGET_USE_NAMESPACE

DrawTool *CCutTool::createTool()
{
    return new CCutTool();
}

CCutTool::CCutTool(QObject *parent)
    : DrawFunctionTool(parent)
    , m_pCutItem(nullptr)
    , m_dragHandle(HandleNode::EInnerType::None)
    , m_bModify(false)
{
    initToolButton();
    connect(this, &CCutTool::toolManagerChanged, this, [ = ](DrawBoardToolMgr * old, DrawBoardToolMgr * cur) {
        Q_UNUSED(old)
        auto board = cur->drawBoard();
        connect(board, &DrawBoard::pageAdded, this, [ = ](Page * added) {
            if (added->scene() != nullptr) {
                m_originSizeMap.insert(added->scene(), added->pageRect());
            }
        });
        connect(board, &DrawBoard::pageRemoved, this, [ = ](Page * removed) {
            m_originSizeMap.remove(removed->scene());
        });
        connect(this, &CCutTool::cutSizeChange, board, &DrawBoard::cutSizeChange);
    });

    setContinued(true);
}

CCutTool::~CCutTool()
{
    m_cutItems.clear();
    qWarning() << "CCutTool deleted !!!!!!!!!!!!!";
}

SAttrisList CCutTool::attributions()
{
    CutItem *pItem = getCutItem(drawBoard()->currentPage()->scene());
    if (pItem != nullptr)
        return pItem->attributions();

    return QList<int>() << ECutToolAttri;
}

QAbstractButton *CCutTool::initToolButton()
{
    QAbstractButton *m_cutBtn = toolButton();
    m_cutBtn->setShortcut(QKeySequence(QKeySequence(Qt::Key_C)));
    setWgtAccesibleName(m_cutBtn, "Crop tool button");
    m_cutBtn->setToolTip(tr("Crop canvas (C)"));
    m_cutBtn->setIconSize(QSize(48, 48));
    m_cutBtn->setFixedSize(QSize(37, 37));
    m_cutBtn->setCheckable(true);
    connect(m_cutBtn, &DToolButton::toggled, m_cutBtn, [ = ](bool b) {
        QIcon icon       = QIcon::fromTheme("ddc_screenshot tool_normal");
        QIcon activeIcon = QIcon::fromTheme("ddc_screenshot tool_active");
        m_cutBtn->setIcon(b ? activeIcon : icon);
    });
    m_cutBtn->setIcon(QIcon::fromTheme("ddc_screenshot tool_normal"));
    return m_cutBtn;
}

void CCutTool::setAttributionVar(int attri, const QVariant &var, int phase, bool autoCmdStack)
{
    if (attri == ECutToolAttri) {
        QList<QVariant> vars = var.toList();
        if (vars.count() != 2) {
            return;
        }
        ECutType cuttp = ECutType(vars.first().toInt());
        QSize sz = vars.at(1).toSize();

        auto scene = drawBoard()->currentPage()->scene();

        //避免手动调整框后，再重复设置rect
        if (sz != m_pCutItem->rect().size().toSize()) {
            this->changeCutType(cuttp, scene);
            this->changeCutSize(scene, sz);
        }
    }
}

int CCutTool::toolType() const
{
    return cut;
}

void CCutTool::funcStart(ToolSceneEvent *event)
{
    Q_UNUSED(event)
    m_pCutItem = getCurCutItem();

    QGraphicsItem *pFirstItem = event->itemsUnderPressedPos().isEmpty() ? nullptr : event->itemsUnderPressedPos().first();
    if (pFirstItem != nullptr) {
        event->view()->page()->setDrawCursor(Qt::ClosedHandCursor);
    }
}

void CCutTool::funcUpdate(ToolSceneEvent *event, int decided)
{
    if (decided == EDragMove) {
        m_pCutItem->move(event->lastEvent()->pos(), event->pos());
    } else if (decided == EResizeMove) {
        HandleNode::EInnerType direction = m_clickHandle;
        m_pCutItem->resizeCutSize(direction, event->lastEvent()->pos(), event->pos(), nullptr);
        emit cutSizeChange(m_pCutItem->rect().size().toSize(), true);
    }
    event->view()->viewport()->update();
}

void CCutTool::funcFinished(ToolSceneEvent *event, int decided)
{
    Q_UNUSED(decided)
    Q_UNUSED(event)
    funHover(event);
}

int CCutTool::funcDecide(ToolSceneEvent *event)
{
    EOperateType ret = ENothing;

    QGraphicsItem *pStartPosTopQtItem = event->scene()->topItem(event->firstEvent()->pos(),
                                                                /*event->itemsUnderPressedPos()*/QList<QGraphicsItem *>(), true, true);

    if (event->scene()->isHandleNode(pStartPosTopQtItem)) {
        HandleNode *pHandle = dynamic_cast<HandleNode *>(pStartPosTopQtItem);
        m_clickHandle = static_cast<HandleNode::EInnerType>(pHandle->nodeType());
        ret = EResizeMove;
    } else if (pStartPosTopQtItem == m_pCutItem) {
        ret = EDragMove;
    }

    return ret;
}

void CCutTool::funHover(ToolSceneEvent *event)
{
    QPointF scenePos = event->pos();

    QList<QGraphicsItem *> items = event->scene()->items(scenePos);

    QGraphicsItem *pItem = event->scene()->topItem(event->pos(), event->itemsUnderPressedPos(), true, true, false, false);//event->scene()->firstItem(scenePos, items, true, true, false, false);

    if (event->scene()->isHandleNode(pItem)) {
        HandleNode *pHandle = dynamic_cast<HandleNode *>(pItem);
        event->view()->page()->setDrawCursor(pHandle->cursor());
    } else if (pItem != nullptr && pItem->type() == CutType) {
        event->view()->page()->setDrawCursor(Qt::OpenHandCursor);
    } else {
        event->view()->page()->setDrawCursor(Qt::ArrowCursor);
    }
}

void CCutTool::createCutItem(PageScene *scene)
{
    if (!m_cutItems.contains(scene)) {
        deleteCutItem(scene);

        scene->clearSelections();

        m_pCutItem = new CutItem(scene->sceneRect());
        scene->addPageItem(m_pCutItem);

        if (m_originSizeMap.find(scene) != m_originSizeMap.end())
            m_pCutItem->setOriginalInitRect(m_originSizeMap[scene]);

        m_dragHandle = HandleNode::EInnerType::None;

        m_pCutItem->setIsFreeMode(true);
        m_pCutItem->setSelected(true);
        m_bModify = false;

        m_cutItems.insert(scene, m_pCutItem);
        emit cutSizeChange(m_pCutItem->rect().size().toSize(), false);

    }
}

void CCutTool::deleteCutItem(PageScene *scene)
{
    if (m_cutItems.contains(scene)) {
        auto itf = m_cutItems[scene];
        CutItem *pCutItem = itf;
        scene->removePageItem(pCutItem);
        if (pCutItem == m_pCutItem) {
            m_pCutItem = nullptr;
        }

        delete pCutItem;
        m_cutItems.remove(scene);
    }
}

QSizeF CCutTool::changeCutType(int type, PageScene *scene)
{
    CutItem *pItem = getCutItem(scene);
    if (pItem != nullptr) {
        if (cut_free == type) {
            pItem->setIsFreeMode(true);
            pItem->setSelected(true);
        } else {
            pItem->setSelected(false);
            pItem->setRatioType(ECutType(type));
        }
        m_bModify = true;
        return pItem->rect().size();
    }
    return QSizeF(0, 0);
}

void CCutTool::changeCutSize(const PageScene *scene, const QSize &size)
{
    CutItem *pItem = getCutItem(const_cast<PageScene *>(scene));
    if (pItem != nullptr) {
        pItem->setSize(size.width(), size.height());
        m_bModify = true;
    }
}

void CCutTool::doFinished(bool accept, bool cmd)
{
    CutItem *pCutItem = getCurCutItem();

    QRectF wantedRect = pCutItem->mapRectToScene(pCutItem->rect());
    qWarning() << pCutItem->originalRect() << wantedRect;
    bool changed = (wantedRect != pCutItem->originalRect());
    if (accept && changed) {
        PageView *pView = drawBoard()->currentPage()->view();
        UndoRecorder block(pView->pageScene()->currentTopLayer(), LayerUndoCommand::RectChanged);
        UndoRecorder block_sence(pView->pageScene()->currentTopLayer(), LayerUndoCommand::SenceRectChanged);
        pView->pageScene()->setSceneRect(wantedRect);
        pView->pageScene()->currentTopLayer()->setRect(wantedRect);
    }
    drawBoard()->setCurrentTool(selection);
}

bool CCutTool::getCutStatus()
{
    CutItem *pCutItem = getCurCutItem();
    PageView *pView = drawBoard()->currentPage()->view();

    if (pCutItem == nullptr || pView == nullptr) {
        qDebug() << "pCutItem or pView is nullptr";
        return false;
    }

    QRectF sceneItemBd = pCutItem->mapRectToScene(pCutItem->rect());

    if (sceneItemBd != pView->sceneRect()) {
        return true;
    } else {
        return false;
    }
}

CutItem *CCutTool::getCurCutItem()
{
    PageView *pView = drawBoard()->currentPage()->view();
    if (pView != nullptr) {
        return getCutItem(pView->pageScene());
    }
    return nullptr;
}

CutItem *CCutTool::getCutItem(PageScene *scene)
{
    if (scene != nullptr) {
        if (m_cutItems.contains(scene)) {
            return m_cutItems[scene];
        } else {
            return nullptr;
        }
    }
    return nullptr;
}

void CCutTool::onStatusChanged(EStatus oldStatus, EStatus newStatus)
{
    if (currentPage() == nullptr)
        return;

    auto page = currentPage();
    if (oldStatus == EIdle && newStatus == EReady) {
        page->installEventFilter(this);
        createCutItem(page->scene());
    } else if (newStatus == EIdle) {
        page->removeEventFilter(this);
        deleteCutItem(page->scene());
    }
}

bool CCutTool::blockPageBeforeOutput(Page *page)
{
    if (page->currentTool_p() == this) {
        drawBoard()->setCurrentPage(page);
        CCutDialog dialog(drawBoard());
        int ret = dialog.exec();
        if (CCutDialog::Save == ret) {
            doFinished(true, true);
            auto view = drawBoard()->currentPage()->view();
            view->pageScene()->pageContext()->setDirty(view->isModified());
        }  else if (CCutDialog::Discard == ret) {
            drawBoard()->setCurrentTool(selection);
        }
    }
    return false;
}

bool CCutTool::eventFilter(QObject *o, QEvent *e)
{
    if (o == drawBoard()->currentPage()) {
        if (e->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvnt = static_cast<QKeyEvent *>(e);
            if (keyEvnt->key() == Qt::Key_Return || keyEvnt->key() == Qt::Key_Enter) {
                doFinished(true, true);
            }
        }
    }
    return DrawFunctionTool::eventFilter(o, e);
}
