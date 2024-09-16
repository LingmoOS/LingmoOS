// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textitem.h"
#include "cgraphicsproxywidget.h"

#include "ctextedit.h"
#include "itemgroup.h"
#include "pagescene.h"
#include "pageview.h"
#include "cgraphicsitemevent.h"
#include "attributewidget.h"

#include <QApplication>

#include <QTextEdit>
#include <QPainter>
#include <QPainterPath>
#include <QTextBlock>
#include <QString>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPointF>
#include <QFont>
#include <QMenu>
#include <QDebug>
#include <QObject>
#include <QTextDocument>
#include <QtMath>

REGISTITEMCLASS(TextItem, TextType)


class TextItem::TextItem_private
{
public:
    explicit TextItem_private(TextItem *qq): q(qq)
        , m_pTextEdit(nullptr)
        , m_pProxy(nullptr)
        , isWrap(true)
    {}
    ~TextItem_private()
    {
        if (m_pTextEdit != nullptr) {
            m_pTextEdit->deleteLater();
            m_pTextEdit = nullptr;
        }
    }
    void initTextEditor(const QString &text = "")
    {
        m_pTextEdit = new TextEdit(q);
        m_pProxy = new ProxyTextWidgetItem(q);
        m_pProxy->setWidget(m_pTextEdit);
        m_pProxy->setMinimumSize(1, 1);
        m_pTextEdit->setText(text);
        changToReadOnlyState();
        m_pTextEdit->selectAll();
        m_pProxy->installEventFilter(m_pTextEdit);
        //m_pTextEdit->setCurrentFontFamily("Noto Sans CJK SC");
        //m_pTextEdit->setHtml(m_pTextEdit->toHtml());
        //qWarning() << "init context ===== " << m_pTextEdit->toHtml();
    }

    void beginPreview()
    {
        if (!_isPreview) {
            _isPreview = true;
            if (m_pTextEdit != nullptr) {
                QTextCursor tCur = m_pTextEdit->textCursor();
                tCur.beginEditBlock();
                m_pTextEdit->setTextCursor(tCur);
                qDebug() << "beginPreview avable undo count = " << m_pTextEdit->document()->availableUndoSteps();
            }
        } else {
            if (m_pTextEdit != nullptr) {
                QTextCursor tCur = m_pTextEdit->textCursor();
                tCur.joinPreviousEditBlock();
                m_pTextEdit->setTextCursor(tCur);
                qDebug() << "joinPreviousEdit avable undo count = " << m_pTextEdit->document()->availableUndoSteps();
            }
        }
    }

    void endPreview(bool revert)
    {
        if (isPreview() && m_pTextEdit != nullptr) {
            QTextCursor tCur = m_pTextEdit->textCursor();
            tCur.endEditBlock();
            m_pTextEdit->setTextCursor(tCur);
            //qDebug() << "endPreview   avable undo count = " << m_pTextEdit->document()->availableUndoSteps();
        }

        if (revert) {
            if (isPreview()) {
                QTextCursor txtCursorBefore = m_pTextEdit->textCursor();
                int begin = txtCursorBefore.selectionStart();
                int end   = txtCursorBefore.selectionEnd();

                if (txtCursorBefore.hasSelection()) { //如果没有选中证明没有修改过,那么就不用还原；否则则还原
                    m_pTextEdit->undo();
                }

                QTextCursor txtCursorAfter = m_pTextEdit->textCursor();
                txtCursorAfter.setPosition(begin);
                txtCursorAfter.setPosition(end, QTextCursor::KeepAnchor);
                m_pTextEdit->setTextCursor(txtCursorAfter);
                //qDebug() << "endPreview1   avable undo count = " << m_pTextEdit->document()->availableUndoSteps();
            }
        }
        _isPreview = false;
    }

    bool isPreview()
    {
        return _isPreview;
    }

    void updateProxyChildItemRect()
    {
        if (m_pProxy != nullptr) {
            m_pProxy->resize(qRound(q->rect().width()), qRound(q->rect().height()));
            updateProxyItemPos();
        }
    }

    void updateProxyItemPos()
    {
        const QRectF &geom = q->itemRect();
        m_pProxy->setPos(geom.x(), geom.y());
    }
    void changToEditState(bool selectAll = true)
    {
        if (q->isMutiSelected() || q->pageGroup() != nullptr)
            return;

        m_pTextEdit->setTextInteractionFlags(m_pTextEdit->textInteractionFlags() | (Qt::TextEditable));

        auto currentTool = q->pageScene() == nullptr ? selection : q->pageScene()->page()->currentTool();
        if (currentTool == selection || currentTool == EDrawToolMode::text) {

            if (m_pTextEdit->isHidden())
                m_pTextEdit->show();

            m_pProxy->setFlag(ItemHasNoContents, false);

            if (selectAll) {
                QTextCursor textCursor = m_pTextEdit->textCursor();
                textCursor.select(QTextCursor::Document);
                m_pTextEdit->setTextCursor(textCursor);
            } else {
                QTextCursor cursor = m_pTextEdit->textCursor();
                cursor.movePosition(QTextCursor::End);
                m_pTextEdit->setTextCursor(cursor);
            }
        }

        //保证被选中
        if (q->pageScene() != nullptr) {
            q->pageScene()->selectPageItem(q);
        }

        //保证自身的焦点
        toFocusEiditor();

        m_pTextEdit->applyDefaultToFirstFormat();

        //进入编辑时,清空当前的撤销重做栈
        m_pTextEdit->document()->clearUndoRedoStacks();
    }

    void changToReadOnlyState(bool selectAll = true)
    {
        if (m_pTextEdit == nullptr)
            return;

        if (selectAll) {
            m_pTextEdit->selectAll();
        }
        m_pTextEdit->document()->clearUndoRedoStacks();

        m_pProxy->setFlag(ItemHasNoContents, true);

        if (q->pageScene() != nullptr)
            q->pageScene()->notSelectPageItem(q);
    }

    void toFocusEiditor()
    {
        if (q->isEditing()) {
            //保证按键响应到textedit控件的底层(从而才能将key事件传递给textedit)
            if (q->pageView() != nullptr) {
                q->pageView()->setFocus();
            }
            //保证控件可编辑
            m_pTextEdit->setTextInteractionFlags(m_pTextEdit->textInteractionFlags() | (Qt::TextEditable));

            //保证自身的焦点
            m_pTextEdit->setFocus();
        }
    }
    void drawDocument(QPainter *painter,
                      const QTextDocument *doc,
                      const QRectF &r = QRectF())
    {
        if (doc->isEmpty())
            return;

        painter->save();
        // 按区域绘制
        if (r.isValid()) {
            painter->setClipRect(r, Qt::IntersectClip);
        }
        painter->translate(r.topLeft());
        QTextDocument *t_doc = const_cast<QTextDocument *>(doc);
        t_doc->drawContents(painter, QRectF());
        painter->restore();
    }
    void updateToDefaultTextFormat()
    {
        m_pTextEdit->selectAll();
        if (q->page() != nullptr) {
            q->setFontFamily(q->page()->defaultAttriVar(EFontFamily).toString());
            q->setFontStyle(q->page()->defaultAttriVar(EFontWeightStyle).toString());
            q->setFontSize(q->page()->defaultAttriVar(EFontSize).toInt());
            q->setTextColor(q->page()->defaultAttriVar(EFontColor).value<QColor>());
        }
    }

    TextEdit             *m_pTextEdit = nullptr;
    ProxyTextWidgetItem  *m_pProxy    = nullptr;

    QFont  m_Font;
    QColor m_color;
    bool   isWrap = true;
    bool   _isPreview      = false;

    bool   modifyed = false;

    TextItem *q;
};
TextItem::TextItem(const QString &text, PageItem *parent)
    : RectBaseItem(parent), TextItem_d(new TextItem_private(this))

{
    d_TextItem()->initTextEditor(text);
}

TextItem::~TextItem()
{
//    delete TextItem_d;
    //    TextItem_d = nullptr;
}

void TextItem::setEditBoxActive()
{
    QTextCursor textCursor = d_TextItem()->m_pTextEdit->textCursor();
    textCursor.select(QTextCursor::Document);
    d_TextItem()->m_pTextEdit->setTextCursor(textCursor);
}

SAttrisList TextItem::attributions()
{
    SAttrisList result;
    //result <<  SAttri(EFontColor, textColor())
    result <<  SAttri(EFontColor, textColor())
           <<  SAttri(EFontFamily, fontFamily())
           <<  SAttri(EFontWeightStyle,  fontStyle())
           <<  SAttri(EFontSize,  fontSize())
           <<  SAttri(ERotProperty, drawRotation());
    return result;
}

void TextItem::setAttributionVar(int attri, const QVariant &var, int phase)
{
    if (phase == EChangedBegin) {
        d_TextItem()->beginPreview();
    }

    switch (attri) {
    case  EFontColor: {
        setTextColor(var.value<QColor>());
        break;
    }
    case  EFontFamily: {
        setFontFamily(var.toString());
        break;
    }
    case  EFontWeightStyle: {
        setFontStyle(var.toString());
        break;
    }
    case  EFontSize: {
        setFontSize(var.toInt());

        // 动态变更时需要主动提醒光标标记的文本已变更,自适应大小
        if (EChangedUpdate == phase) {
            textEditor()->markCursorDataDirty();
        }
        break;
    }
    default:
        break;
    }
    if (phase == EChangedFinished) {
        d_TextItem()->endPreview(false);
    }

    if (phase == EChangedAbandon) {
        d_TextItem()->endPreview(true);
    }
    update();
}

QPainterPath TextItem::highLightPath() const
{
    QPainterPath path;
    path.addRect(this->itemRect());
    return path;
}

QTextCharFormat TextItem::currentCharFormat() const
{
    if (textEditor() != nullptr)
        return textEditor()->currentFormat();

    return QTextCharFormat();
}

void TextItem::setEditing(bool state, bool selectAllText)
{
    if (isEditing())
        return;

    if (state) {
        d_TextItem()->changToEditState(selectAllText);
    } else {
        d_TextItem()->changToReadOnlyState(selectAllText);
    }
}

TextEdit *TextItem::textEditor() const
{
    return d_TextItem()->m_pTextEdit;
}

int TextItem::type() const
{
    return TextType;
}

void TextItem::setText(const QString &text)
{
    if (d_TextItem()->m_pTextEdit != nullptr) {
        d_TextItem()->m_pTextEdit->setText(text);
    }
}

QString TextItem::text() const
{
    if (d_TextItem()->m_pTextEdit != nullptr) {
        return d_TextItem()->m_pTextEdit->toPlainText();
    }
    return "";
}

QFont TextItem::font() const
{
    return textEditor()->currentFont();
}

void TextItem::setFont(const QFont &font)
{
    textEditor()->setCurrentFont(font);
}

QString TextItem::fontStyle()
{
    return textEditor()->currentFontStyle();
}

void TextItem::setFontStyle(const QString &style)
{
    textEditor()->setCurrentFontStyle(style);
}

void TextItem::setFontSize(int size)
{
    textEditor()->setCurrentFontSize(size);
}

QVariant TextItem::fontSize()
{
    if (!isEditing()) {
        qreal currentSize = 0;
        auto doc = textEditor()->document();
        auto currentBlock = doc->begin(); //  QTextBlock 对象
        while (currentBlock.isValid()) { // 遍历所有block
            qreal curFontSize = 0;
            for (auto it = currentBlock.begin(); !(it.atEnd()); it++) { // 遍历block中的内容
                QTextFragment currentFragment = it.fragment(); // QTextFragment 对象
                QTextFormat format = currentFragment.charFormat();
                curFontSize = format.toCharFormat().fontPointSize();

                if (qRound(currentSize) == 0) {
                    currentSize = curFontSize;
                }

                if (qRound(currentSize) != qRound(curFontSize)) {
                    return QVariant();
                }

            }
            currentBlock = currentBlock.next();
        }

        return currentSize;

    } else {
        //多种字体大小会返回0
        return textEditor()->currentFontSize() <= 0 ? QVariant() : textEditor()->currentFontSize();
    }

}

void TextItem::setFontFamily(const QString &family)
{
    textEditor()->setCurrentFontFamily(family);
}

QString TextItem::fontFamily() const
{
    return textEditor()->currentFontFamily();
}

void TextItem::loadUnit(const Unit &data)
{
    TextUnitData i = data.data.data.value<TextUnitData>();
    auto pTextData = &i;

    loadHeadData(data.head);

    if (pTextData != nullptr) {
        //loadGraphicsRectUnit(pTextData->rect);

        QSignalBlocker blocker(textEditor());
        QRectF rect(pTextData->rect.topLeft, pTextData->rect.bottomRight);

        //setFont(pTextData->font);

        //手动调整过大小就意味着不需要自动根据文本调整大小了
        setWrap(!pTextData->manResizeFlag);

        textEditor()->setHtml(pTextData->content);

        //setTextColor(pTextData->color);

        textEditor()->applyDefaultToFirstFormat();

        setRect(rect);

        this->update();
    }
}

void TextItem::setTextColor(const QColor &col)
{
    textEditor()->setCurrentColor(col);
}

QColor TextItem::textColor() const
{
    //return m_color;
    return textEditor()->currentColor();
}

void TextItem::setCurrentFormat(const QTextCharFormat &format, bool merge)
{
    textEditor()->setCurrentFormat(format, merge);
}

void TextItem::paintSelf(QPainter *painter, const QStyleOptionGraphicsItem *option)
{
    //0.会有一种情况就是图元从场景删除后,Qt依旧会掉用到paint函数,这是一个BUG,
    //a.怀疑是修改图元大小时,没有添加prepareGeometryChange()导致
    //b也可能是代理图元proyxy的添加删除交互?
    //当前通过该方法让其不会显示出来
    if (scene() == nullptr) {
        qWarning() << "----------- scene == nullptr but paint !!!! ";
        preparePageItemGeometryChange();
        setRect(QRectF(0, 0, 0, 0));
        return;
    }

    //1.矩形大小不正确不绘制
    if (!itemRect().isValid())
        return;

    beginCheckIns(painter);
    if (!isEditing()) {
        d_TextItem()->drawDocument(painter, textEditor()->document(), this->itemRect());
    }

//    if (isSingleSelected()) {
//        QPen pen;
//        pen.setWidthF(1 / option->levelOfDetailFromTransform(painter->worldTransform()));
//        pen.setColor(QColor(187, 187, 187));

//        painter->setPen(pen);
//        painter->setBrush(QBrush(Qt::NoBrush));
//        painter->drawRect(this->itemRect());
//    }
    //qWarning() << "current family ====== " << fontFamily();

    endCheckIns(painter);
    paintMutBoundingLine(painter, option);
}

QPainterPath TextItem::calOrgShapeBaseRect(const QRectF &rect) const
{
    QPainterPath path;
    path.addRect(rect);
    return path;
}

bool TextItem::isPosPenetrable(const QPointF &posLocal)
{
    Q_UNUSED(posLocal)
    return false;
}

void TextItem::doScalBegin(PageItemScalEvent *event)
{
    Q_UNUSED(event)
    setWrap(false);
}

QVariant TextItem::pageItemChange(int change, const QVariant &value)
{
    if (change == PageItemRectChanged) {
        //1.修改文字编辑控件的大小(通过代理图元)
        d_TextItem()->updateProxyChildItemRect();
    }
    return PageItem::pageItemChange(change, value);
}


bool TextItem::isWrap() const
{
    return d_TextItem()->isWrap;
}

Unit TextItem::getUnit(int reson) const
{
    //qWarning() << "TextItem::getUnit(const Unit &data)";
    Unit unit;
    unit.usage = reson;
    unit.head.dataType = this->type();
    unit.head.dataLength = sizeof(TextUnitData);
    unit.head.pos = this->pos();
    unit.head.rotate = this->drawRotation();
    unit.head.zValue = this->zValue();
    unit.head.trans = this->transform();

    TextUnitData i;
    i.rect.topLeft = itemRect().topLeft();
    i.rect.bottomRight = itemRect().bottomRight();
    i.font = font();
    i.manResizeFlag = !isWrap();
    i.content = textEditor()->toHtml();
    i.color = textColor();
    unit.data.data.setValue<TextUnitData>(i);

    return  unit;
}

bool TextItem::isEditing() const
{
    if (textEditor() == nullptr) {
        return  false;
    }
    bool isediting = (d_TextItem()->m_pProxy->flags() & ItemHasNoContents) ? false : true;
    //不在编辑状态不显示代理框
    if (!isediting)
        d_TextItem()->m_pProxy->setVisible(false);
    return isediting;
}

bool TextItem::isModified() const
{
    if (textEditor() == nullptr) {
        return false;
    }
    return textEditor()->document()->isUndoAvailable();
}

void TextItem::cut()
{
    textEditor()->cut();
}

void TextItem::copy()
{
    textEditor()->copy();
}

void TextItem::paste()
{
    textEditor()->paste();
}

void TextItem::selectAll()
{
    textEditor()->selectAll();
}

void TextItem::setAlignment(const Qt::Alignment &align)
{
    textEditor()->setAlignment(align);
}

void TextItem::undo()
{
    textEditor()->undo();
}

void TextItem::redo()
{
    textEditor()->redo();
}

void TextItem::deleteChar()
{
    textEditor()->textCursor().deleteChar();
}

void TextItem::setInnerUndoEnable(bool b)
{
    if (textEditor() != nullptr) {
        textEditor()->setUndoRedoEnabled(b);
    }
}

bool TextItem::isInnerUndoEnable() const
{
    if (textEditor() != nullptr) {
        textEditor()->isUndoRedoEnabled();
    }
    return false;
}

void TextItem::clearInnerUndoStack()
{
    if (textEditor() != nullptr) {
        textEditor()->document()->clearUndoRedoStacks();
    }
}

void TextItem::setWrap(bool b)
{
    if (b == d_TextItem()->isWrap)
        return;

    d_TextItem()->isWrap = b;

    if (d_TextItem()->isWrap && textEditor() != nullptr)
        textEditor()->setLineWrapMode(QTextEdit::NoWrap);
    else {
        textEditor()->setLineWrapMode(QTextEdit::WidgetWidth);
    }
}

