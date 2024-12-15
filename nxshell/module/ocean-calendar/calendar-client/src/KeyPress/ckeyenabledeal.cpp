// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ckeyenabledeal.h"
#include "cgraphicsscene.h"
#include "scheduledlg.h"
#include "graphicsItem/cscenebackgrounditem.h"
#include "graphicsItem/draginfoitem.h"
#include "myscheduleview.h"

#include <QGraphicsView>

CKeyEnableDeal::CKeyEnableDeal(QGraphicsScene *scene)
    : CKeyPressDealBase(Qt::Key_Return, scene)
{
}

/**
 * @brief CKeyEnableDeal::focusItemDeal     焦点项处理
 * @param item
 * @param scene
 * @return
 */
bool CKeyEnableDeal::focusItemDeal(CSceneBackgroundItem *item, CGraphicsScene *scene)
{
    bool result = false;
    CFocusItem *focusItem = item->getFocusItem();
    if (focusItem != nullptr) {
        result = true;
        QWidget *parentWidget = scene->views().at(0);
        switch (focusItem->getItemType()) {
        case CFocusItem::CBACK: {
            CSceneBackgroundItem *backgroundItem = dynamic_cast<CSceneBackgroundItem *>(focusItem);
            if (backgroundItem != nullptr) {
                QDateTime createDateTime;
                //设置创建时间
                createDateTime.setDate(backgroundItem->getDate());
                createDateTime.setTime(QTime(0, 0, 0));
                //如果为月视图背景则根据是否为当前时间设置不一样的创建时间
                if (backgroundItem->getItemOfView() == CSceneBackgroundItem::OnMonthView) {
                    QDateTime currentDateTime = QDateTime::currentDateTime();
                    //如果为当前时间则设置创建开始时间为当前时间
                    if (backgroundItem->getDate() == currentDateTime.date()) {
                        createDateTime.setTime(currentDateTime.time());
                    } else {
                        createDateTime.setTime(QTime(8, 0, 0));
                    }
                }
                createSchedule(createDateTime, parentWidget);
            }
        } break;
        case CFocusItem::CITEM: {
            DragInfoItem *scheduleItem = dynamic_cast<DragInfoItem *>(focusItem);
            CMyScheduleView dlg(scheduleItem->getData(), parentWidget);
            dlg.exec();
        } break;
        default: {
            scene->signalGotoDayView(focusItem->getDate());
        } break;
        }
    }
    return result;
}

/**
 * @brief CKeyEnableDeal::createSchedule        创建日程
 * @param createDate
 * @param parent
 */
void CKeyEnableDeal::createSchedule(const QDateTime &createDate, QWidget *parent)
{
    CScheduleDlg dlg(1, parent);
    dlg.setDate(createDate);
    dlg.setAllDay(true);
    dlg.exec();
}
