// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cgraphicsproxywidget.h"
#include "cundoredocommand.h"
#include "textitem.h"
#include "pageview.h"
#include "pagescene.h"
#include "ctextedit.h"
#include <QGraphicsSceneContextMenuEvent>
#include <QTextEdit>
#include <QLineEdit>
#include <QDebug>
#include <QApplication>

ProxyWidgetItem::ProxyWidgetItem(PageItem *parent, Qt::WindowFlags wFlags)
    : QGraphicsProxyWidget(parent, wFlags)
{
}

PageItem *ProxyWidgetItem::parentDrawItem()
{
    return dynamic_cast<PageItem *>(parentItem());
}

bool ProxyWidgetItem::event(QEvent *event)
{
//    if (event->type() == QEvent::ThreadChange) {
//        if (widget() != nullptr) {
//            qWarning() << "is main thread ====== " << (this->thread() == qApp->thread())
//                       << (QThread::currentThread() == qApp->thread()) << (widget()->thread() == qApp->thread())
//                       << (QThread::currentThread() == widget()->thread()) ;
//            auto w = widget();
//            //QMetaObject::invokeMethod(this,[=](){});
//            widget()->moveToThread(qApp->thread());
//        }
//    }
    return QGraphicsProxyWidget::event(event);
}

void ProxyWidgetItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    //qWarning() << "ProxyWidgetItem::contextMenuEvent ===== " << event;

    QGraphicsProxyWidget::contextMenuEvent(event);
    //event->ignore();
}

ProxyTextWidgetItem::ProxyTextWidgetItem(PageItem *parent, Qt::WindowFlags wFlags):
    ProxyWidgetItem(parent, wFlags)
{
    setObjectName("ProxyTextWidgetItem");
    setCursor(QCursor(Qt::IBeamCursor));
}

void ProxyTextWidgetItem::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event)
    //如果不具有内容那么不设置焦点
    if (this->flags() & ItemHasNoContents) {
        qWarning() << "focusInEvent setFocusItem nullptr";
        scene()->setFocusItem(nullptr);
    }
}

void ProxyTextWidgetItem::focusOutEvent(QFocusEvent *event)
{
    QGuiApplication::inputMethod()->reset();

    if (!(this->flags() & ItemHasNoContents)) {
        if (parentDrawItem() && parentDrawItem()->pageScene()) {

            setFlag(ItemHasNoContents, true);
            //qWarning() << "not selected focus out !!!!!!!!!" << event->reason();
            parentDrawItem()->pageScene()->notSelectPageItem(parentDrawItem());
        }
    }
    return ProxyWidgetItem::focusOutEvent(event);
}

void ProxyTextWidgetItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    ProxyWidgetItem::paint(painter, option, widget);
}

QVariant ProxyTextWidgetItem::itemChange(QGraphicsItem::GraphicsItemChange doChange, const QVariant &value)
{
    auto result = ProxyWidgetItem::itemChange(doChange, value);
    if (doChange == ItemFlagsHaveChanged) {
        auto parentDrawItem = this->parentDrawItem();
        if (parentDrawItem != nullptr && parentDrawItem->pageScene() != nullptr) {
            auto tool = parentDrawItem->pageScene()->page()->currentTool();
            if (tool == text) {
                //这是初始化添加一个文本，不用进行undo数据收集
                return value;
            }

            static QSizeF s_size = QSizeF(0, 0);
            int flags = value.toInt();
            bool show = !(flags & ItemHasNoContents);
            //qWarning() << QString("go %1 proxy widget (will record parent item data for %2) = ").arg(show ? "show" : "hide").arg(show ? "undo" : "redo") << show;
            if (show) {
                //1.显示出来证明开始进入编辑状态,进入编辑状态前收集当前文本的信息以用于外部撤销
                UndoStack::recordUndo(parentDrawItem);
                s_size = parentDrawItem->itemRect().size();
            } else {
                //2.隐藏内容意味着退出编辑,那么这个时候判断内部文本是否可撤销,如果可撤销证明修改过,那么就收集还原信息
                bool pushToStash = false;
                if (qobject_cast<TextEdit *>(widget()) != nullptr) {
                    TextEdit *pTextEditor = qobject_cast<TextEdit *>(widget());
                    pushToStash = pTextEditor->document()->isUndoAvailable();
                }
                if (!pushToStash) {
                    pushToStash = (s_size != parentDrawItem->itemRect().size());
                }
                if (pushToStash) {
                    UndoStack::recordRedo(parentDrawItem);
                }
                UndoStack::finishRecord(parentDrawItem->pageView()->stack());
            }
        }
    }
    return result;
}
