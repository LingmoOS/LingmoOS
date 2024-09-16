// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "drawboard.h"
#include "drawboardattrimgr.h"
#include "drawboardtoolmgr.h"

DrawBoardAttriMgr::DrawBoardAttriMgr(DrawBoard *db, EDisplayType tp): AttributionManagerWgt(db)
{
    setDrawBoard(db);
    changeWindowTypeTo(tp);
}

void DrawBoardAttriMgr::setDrawBoard(DrawBoard *db)
{
    if (_db != db) {

        if (_db != nullptr) {
            disconnect(_db, &DrawBoard::toolManagerChanged, this, &DrawBoardAttriMgr::_onToolManagerChanged);
        }

        db->setAttributionManager(this);

        _db = db;

        connect(_db, &DrawBoard::toolManagerChanged, this, &DrawBoardAttriMgr::_onToolManagerChanged);
    }
}

DrawBoard *DrawBoardAttriMgr::drawBoard() const
{
    return _db;
}

void DrawBoardAttriMgr::setDisplayType(EDisplayType tp)
{
    if (_tp != tp) {
        changeWindowTypeTo(tp);
    }
}

DrawBoardAttriMgr::EDisplayType DrawBoardAttriMgr::displayType() const
{
    return _tp;
}

void DrawBoardAttriMgr::changeLayout(QLayout *lay)
{
    if (layout() != nullptr) {
        delete  layout();
    }
    setLayout(lay);
}

void DrawBoardAttriMgr::showAt(int active, const QPoint &pos)
{
    if (active == ToolActivedAndClicked) {
        qWarning() << "show at ==== " << pos;
        this->move(pos.x() + 20, pos.y() + 10);
        this->show();
    } /*else {
        this->show();
    }*/
    this->show();
}

void DrawBoardAttriMgr::showWidgets(int active, const QList<QWidget *> &oldWidgets, const QList<QWidget *> &exposeWidgets)
{
    refreshLayout(active, oldWidgets, exposeWidgets);
}

void DrawBoardAttriMgr::paintEvent(QPaintEvent *event)
{
    AttributionManagerWgt::paintEvent(event);
}

void DrawBoardAttriMgr::changeWindowTypeTo(EDisplayType tp)
{
    Qt::WindowFlags flags;

    int flg = showFlags();

    switch (tp) {
    case EPopupForToolActive: {
        flags |= Qt::Popup;
        flg = ToolActivedAndClicked;
        break;
    }
    case ETool: {
        flags |= Qt::Tool | Qt::WindowStaysOnTopHint;
        flg = ToolActived | ItemSelected;
        raise();
        break;
    }
    case EWidget: {
        flags = Qt::Widget;
        flg = ToolActived | ItemSelected;
        break;
    }
    case EAlwaysHide: {
        flags |= Qt::Widget;
        flg = NoDisplay;
        break;
    }
    }
    setWindowFlags(flags);
    setShowFlags(flg);

    _tp = tp;

    //refreshLayout(EInnerUpdate, QList<QWidget *>(), exposeWidgets);
}

void DrawBoardAttriMgr::refreshLayout(int active, const QList<QWidget *> &oldWidgets, const QList<QWidget *> &exposeWidgets)
{
    switch (_tp) {
    case EPopupForToolActive: {
        hide();
        break;
    }
    case ETool: {
        show();
        raise();
        break;
    }
    case EWidget: {
        setVisible(!exposeWidgets.isEmpty());
        break;
    }
    case EAlwaysHide: {
        hide();
        return;
    }
    }

    foreach (auto w, oldWidgets) {
        w->setParent(nullptr);
        w->hide();
    }
    foreach (auto w, exposeWidgets) {
        w->setParent(this);
        layout()->addWidget(w);
        w->show();
    }
}

void DrawBoardAttriMgr::displayForPopup()
{
//    auto btn = qobject_cast<QAbstractButton *>(sender());
//    if (_db->currentTool_p() != nullptr && _db->currentTool_p()->toolButton() == btn) {
//        auto rect = this->sizeHint();

//        QPoint pos;
//        auto toolMgrWgtSize = _db->toolManager()->size();

//        if (toolMgrWgtSize.width() < toolMgrWgtSize.height()) {
//            pos = btn->mapToGlobal(QPoint(btn->width() + 6, 0));
//            pos.setY(pos.y() - (rect.height() - btn->height()) / 2);
//        } else {
//            pos = btn->mapToGlobal(QPoint(0, - rect.height() - 6));
//            pos.setX(pos.x() - (rect.width() - btn->width()) / 2);
//        }

//        this->move(pos);
//        this->show();
//    }
}

void DrawBoardAttriMgr::_onToolManagerChanged(DrawBoardToolMgr *pOldMgr, DrawBoardToolMgr *pNowMgr)
{
    if (_tp == EPopupForToolActive) {
        if (pOldMgr != nullptr) {
            auto tools = pOldMgr->allTools();
//            foreach (auto tool, tools) {
//                disconnect(tool->toolButton(), &QAbstractButton::released, this, &DrawBoardAttriMgr::displayForPopup);
//            }
        }

        if (pNowMgr != nullptr) {
            auto tools = pNowMgr->allTools();
//            foreach (auto tool, tools) {
//                connect(tool->toolButton(), &QAbstractButton::released, this, &DrawBoardAttriMgr::displayForPopup);
//            }
        }
    }
}
