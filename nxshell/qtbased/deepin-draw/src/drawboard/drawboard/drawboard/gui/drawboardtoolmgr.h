// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RIGHTTOOLBAR_H
#define RIGHTTOOLBAR_H

#include "adaptivedef.h"
#include "globaldefine.h"
#include "pageitem.h"
#include "drawtoolfactory.h"

#include <QFrame>
#include <QWidget>
#include <QPushButton>
#include <QToolButton>


class QVBoxLayout;
class PushButton;

class DrawBoard;


#ifdef USE_DTK
#include <DFrame>
DWIDGET_USE_NAMESPACE
#endif

class DRAWLIB_EXPORT DrawBoardToolMgr : public QWidget
{
    Q_OBJECT
public:
    DrawBoardToolMgr(DrawBoard *db, QWidget *parent = nullptr);
    ~DrawBoardToolMgr();

    DrawBoard *drawBoard() const;

    int  currentTool()const;
    bool setCurrentTool(int tool, bool force = false);
    bool setCurrentTool(DrawTool *tool, bool force = false);
    DrawTool *tool(int tool)const;

    bool addTool(int tool);
    bool addTool(DrawTool *tool);

    void removeTool(int tool);
    void removeTool(DrawTool *tool);

    void addDefaultTools();
    void addPluginTools();
    void clearAllTools(bool dlt = true);

    QMap<int, DrawTool *> allTools() const;

    void changeButtonLayout(QLayout *layout);
    QLayout *buttonLayout()const;

signals:
    void currentToolChanged(int oldTool, int nowTool);
    void toolClicked(int tool);

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;                      //进入QWidget瞬间事件
    void paintEvent(QPaintEvent *event) override;

private:
    PRIVATECLASS(DrawBoardToolMgr)
};

#endif // RIGHTTOOLBAR_H
