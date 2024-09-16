// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "texttool.h"
#include "pagescene.h"
#include "textitem.h"
#include "pagecontext.h"
#include "ctextedit.h"
#include "pageview.h"
#include "cundoredocommand.h"

#include <QToolButton>
#include <QStack>

class TextTool::TextTool_private
{
public:
    explicit TextTool_private(TextTool *qq): q(qq) {}

    TextTool *q;

    void pushOneText(TextItem *item)
    {
        if (addedItems.isEmpty() || addedItems.top() != item) {
            recordLastCreate();
            item->clearInnerUndoStack();
            addedItems.push(item);
        }
    }
    TextItem *popOneText(const QString &text)
    {
        if (!addedItems.isEmpty()) {
            auto it = addedItems.top();
            if (it->isModified()) {
                return new TextItem(text);
            } else {
                it = addedItems.pop();
                it->textEditor()->selectAll();
                it->clearInnerUndoStack();
                return it;
            }
        }
        return new TextItem(text);
    }

    void recordLastCreate()
    {
        //下一个item入栈，记录上一个
        if (!addedItems.isEmpty() && nullptr != addedItems.top()) {
            if (q->isContinued()) {
                if (addedItems.top()->isModified()) {
                    UndoRecorder recoder(addedItems.top()->layer(), LayerUndoCommand::ChildItemAdded, addedItems.top());
                } else {
                    //没有做修改的在结束时，需要删掉
                    auto pItem = addedItems.pop();
                    auto pScene = dynamic_cast<PageScene *>(pItem->scene());

                    if (nullptr != pScene) {
                        pScene->removePageItem(pItem);
                    }
                    delete pItem;
                }
            } else {
                UndoRecorder recoder(addedItems.top()->layer(), LayerUndoCommand::ChildItemAdded, addedItems.top());
            }
        }
    }

    QStack<TextItem *> addedItems;
};


TextTool::TextTool(QObject *parent)
    : DrawItemTool(parent), TextTool_d(new TextTool_private(this))
{
    setAutoPushToundoStack(false);

    setAutoCheckItemDeleted(false);

    setMaxTouchPoint(1);

    //setContinued(true);

    setCursor(QCursor(Qt::IBeamCursor));

    auto m_textBtn = toolButton();
    m_textBtn->setShortcut(QKeySequence(QKeySequence(Qt::Key_T)));
    setWgtAccesibleName(m_textBtn, "Text tool button");
    m_textBtn->setToolTip(tr("Text (T)"));
    m_textBtn->setIconSize(TOOL_ICON_RECT);
    m_textBtn->setFixedSize(TOOL_BUTTON_RECT);
    m_textBtn->setCheckable(true);
    m_textBtn->setIcon(QIcon::fromTheme("text_normal"));
    connect(m_textBtn, &QToolButton::toggled, m_textBtn, [ = ](bool b) {
        QIcon icon       = QIcon::fromTheme("text_normal");
        QIcon activeIcon = QIcon::fromTheme("text_highlight");
        m_textBtn->setIcon(b ? activeIcon : icon);
    });
}

TextTool::~TextTool()
{

}

int TextTool::toolType() const
{
    return text;
}

SAttrisList TextTool::attributions()
{
    SAttrisList result;
    result << defaultAttriVar(EFontColor)
           << defaultAttriVar(EFontFamily)
           << defaultAttriVar(EFontWeightStyle)
           << defaultAttriVar(EFontSize)
           << defaultAttriVar(ERotProperty);
    return result;
}

void TextTool::setAttributionVar(int attri, const QVariant &var, int phase, bool autoCmdStack)
{
    DrawItemTool::setAttributionVar(attri, var, phase, autoCmdStack);
}

void TextTool::drawItemFinish(ToolSceneEvent *event, PageItem *p)
{
    TextItem *pItem = dynamic_cast<TextItem *>(p);
    if (nullptr != pItem) {
        //pItem->textEditor()->applyDefaultToFirstFormat();
        //only one allow in edit. so if support count more than 1,all new text should not be in edit status.
        if (maxTouchPoint() == 1) {
            //在创建完成会切换工具，会取消编辑状态，再次设置确保进入修改状态
            QMetaObject::invokeMethod(this, [ = ] {
                pItem->setEditing(true, true);
            }, Qt::QueuedConnection);
        }
        d_TextTool()->pushOneText(pItem);
    }
}

void TextTool::onStatusChanged(EStatus oldStatus, EStatus nowStatus)
{
    if (oldStatus == EReady && nowStatus == EIdle) {
        d_TextTool()->recordLastCreate();
        //        qWarning() << "d_TextTool()->addedItems count ===== " << d_TextTool()->addedItems.count();
        //        UndoRecorder recorder(drawBoard()->currentPage()->scene()->currentTopLayer(),
        //                              LayerUndoCommand::ChildItemAdded,
        //                              switchListClassTo<TextItem, PageItem>(d_TextTool()->addedItems.toList()));
    } else if (oldStatus == EIdle && nowStatus == EReady) {
        d_TextTool()->addedItems.clear();
    }
}

bool TextTool::isPressEventHandledByQt(ToolSceneEvent *event)
{
    //如果是代理的widget那么需要传递可能需要传递事件给Qt处理(处理焦点丢失等)
    if (event->view()->activeProxWidget() != nullptr) {
        //节点node不用传递给qgraphics的qt框架 自己处理调整图元大小
        auto itemsUnderPos = event->itemsUnderPressedPos();
        QGraphicsItem *pItem = itemsUnderPos.isEmpty() ? nullptr : itemsUnderPos.first();
        if (event->scene()->isHandleNode(pItem)) {
            return false;
        }
        auto activeItem = event->view()->activeProxDrawItem();
        return activeItem->contains(event->toItemPos(activeItem));
    }
    return false;
}

PageItem *TextTool::drawItemStart(ToolSceneEvent *event)
{
    if (event->isNormalPressed()) {
        TextItem *pItem = d_TextTool()->popOneText(tr("Input text here"));
        pItem->setPos(event->currentLayerPos());
        return pItem;
    }
    return nullptr;
}

int TextTool::minMoveUpdateDistance()
{
    return 0;
}
