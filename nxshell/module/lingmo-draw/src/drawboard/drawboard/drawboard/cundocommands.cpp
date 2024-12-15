// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cundocommands.h"
#include "globaldefine.h"
#include "pagecontext.h"
#include "pagescene.h"
#include "pageview.h"

#include <QUndoCommand>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QBrush>
#include <QDebug>

CSceneCutCommand::CSceneCutCommand(PageScene *scene, QRectF rect, QUndoCommand *parent, PageItem *item)
    : QUndoCommand(parent)
    , m_newRect(rect)
    , m_oldRect(scene->sceneRect())
{
    myGraphicsScene = scene;
    m_item = item;
}

CSceneCutCommand::~CSceneCutCommand()
{
}

void CSceneCutCommand::undo()
{
    myGraphicsScene->setSceneRect(m_oldRect);
    myGraphicsScene->update();
}

void CSceneCutCommand::redo()
{
    myGraphicsScene->setSceneRect(m_newRect);
    myGraphicsScene->update();
}

CItemsAlignCommand::CItemsAlignCommand(PageScene *scene, QMap<PageItem *, QPointF> startPos,
                                       QMap<PageItem *, QPointF> endPos)
    : myGraphicsScene(scene)
    , m_itemsStartPos(startPos)
    , m_itemsEndPos(endPos)
    , m_isMoved(true)
{
}

void CItemsAlignCommand::undo()
{
    qDebug() << "CItemsAlignCommand::undo";
    if (m_isMoved) {
        QList<PageItem *> allItems = m_itemsStartPos.keys();
        QList<QPointF> itemsStartPos = m_itemsStartPos.values();
        QList<QPointF> itemsEndPos = m_itemsEndPos.values();

        for (int i = 0; i < allItems.size(); i++) {
            allItems.at(i)->move(itemsEndPos.at(i), itemsStartPos.at(i));
        }
    }
    m_isMoved = false;
}

void CItemsAlignCommand::redo()
{
    qDebug() << "CItemsAlignCommand::redo";
    if (!m_isMoved) {
        QList<PageItem *> allItems = m_itemsStartPos.keys();
        QList<QPointF> itemsStartPos = m_itemsStartPos.values();
        QList<QPointF> itemsEndPos = m_itemsEndPos.values();

        for (int i = 0; i < allItems.size(); i++) {
            allItems.at(i)->move(itemsStartPos.at(i), itemsEndPos.at(i));
        }
    }
    m_isMoved = true;
}
