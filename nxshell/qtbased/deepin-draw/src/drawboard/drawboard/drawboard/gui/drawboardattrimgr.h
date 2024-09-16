// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DRAWBOARDATTRIMGRWGT_H
#define DRAWBOARDATTRIMGRWGT_H
#include "attributemanager.h"

class DrawBoard;
class DrawBoardToolMgr;
class DRAWLIB_EXPORT DrawBoardAttriMgr: public AttributionManagerWgt
{
    Q_OBJECT
public:
    enum EDisplayType {EPopupForToolActive, ETool, EWidget, EAlwaysHide};

    DrawBoardAttriMgr(DrawBoard *db = nullptr, EDisplayType tp = EPopupForToolActive);

    void setDrawBoard(DrawBoard *db);
    DrawBoard *drawBoard() const;

    void setDisplayType(EDisplayType tp);
    EDisplayType displayType() const;

    void changeLayout(QLayout *lay);

protected:
    void showAt(int active, const QPoint &pos) override;
    void showWidgets(int active,
                     const QList<QWidget *> &oldWidgets, const QList<QWidget *> &exposeWidgets) override;

    void paintEvent(QPaintEvent *event) override;

private:
    void changeWindowTypeTo(EDisplayType tp);
    void refreshLayout(int active, const QList<QWidget *> &oldWidgets, const QList<QWidget *> &exposeWidgets);

    Q_SLOT void displayForPopup();

    Q_SLOT void _onToolManagerChanged(DrawBoardToolMgr *pOldMgr, DrawBoardToolMgr *pNowMgr);

    DrawBoard *_db = nullptr;
    EDisplayType _tp = EPopupForToolActive;
};

#endif // DRAWBOARDATTRIMGRWGT_H
