// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cgraphicstextitem.h"
#include "cgraphicsproxywidget.h"
//#include "cgraphicsitemhighlight.h"
#include "cgraphicsitem.h"
#include "widgets/ctextedit.h"
#include "cdrawscene.h"
#include "frame/cviewmanagement.h"
#include "frame/cgraphicsview.h"
#include "cgraphicsitemselectedmgr.h"
#include "application.h"
#include "cgraphicsitemevent.h"
#include "cattributeitemwidget.h"
#include "../../../service/dyncreatobject.h"

#include <DApplication>

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

REGISTITEMCLASS(CGraphicsTextItem, TextType)
CGraphicsTextItem::CGraphicsTextItem(const QString &text, CGraphicsItem *parent)
    : CGraphicsRectItem(parent)
    , m_pTextEdit(nullptr)
    , m_pProxy(nullptr)
    , _autoAdjustSize(true)
{
    initTextEditor(text);
}

CGraphicsTextItem::~CGraphicsTextItem()
{
    if (m_pTextEdit != nullptr) {
        m_pTextEdit->deleteLater();
        m_pTextEdit = nullptr;
    }
}

DrawAttribution::SAttrisList CGraphicsTextItem::attributions()
{
    DrawAttribution::SAttrisList result;
    result << DrawAttribution::SAttri(DrawAttribution::EFontColor, textColor())
           << DrawAttribution::SAttri(DrawAttribution::EFontFamily, fontFamily())
           << DrawAttribution::SAttri(DrawAttribution::EFontWeightStyle,  fontStyle())
           << DrawAttribution::SAttri(DrawAttribution::EFontSize,  fontSize());
    return result;
}

void CGraphicsTextItem::setAttributionVar(int attri, const QVariant &var, int phase)
{
//    if (phase == EChangedBegin) {
//        this->beginPreview();
//    }
    switch (attri) {
    case DrawAttribution::EFontColor: {
        setTextColor(var.value<QColor>());
        break;
    }
    case DrawAttribution::EFontFamily: {
        setFontFamily(var.toString());
        break;
    }
    case DrawAttribution::EFontWeightStyle: {
        setFontStyle(var.toString());
        break;
    }
    case DrawAttribution::EFontSize: {
        setFontSize(var.toInt());
        break;
    }
    default:
        break;
    }
//    if (phase == EChangedFinished) {
//        this->endPreview();
//    }
    update();
}

void CGraphicsTextItem::initTextEditor(const QString &text)
{
    m_pTextEdit = new CTextEdit(this);
    m_pTextEdit->setText(text);
    m_pTextEdit->setMinimumSize(QSize(1, 1));

    m_pTextEdit->setWindowFlags(Qt::FramelessWindowHint);
    m_pTextEdit->setFrameShape(QTextEdit::NoFrame);
    m_pTextEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pProxy = new CGraphicsProxyWidget(this);
    m_pProxy->setWidget(m_pTextEdit);
    m_pProxy->setMinimumSize(0, 0);
    m_pProxy->setZValue(this->zValue() - 0.1);
    m_pTextEdit->setFocus();

    enum EDirection { LeftTop, Top, RightTop, Right, RightBottom, Bottom, LeftBottom, Left, Rotation, InRect, None};

    //全选会更改一次字体 所以字体获取要在这之前
    QTextCursor textCursor = m_pTextEdit->textCursor();
    textCursor.select(QTextCursor::Document);
    m_pTextEdit->setTextCursor(textCursor);

    changToReadOnlyState();
    m_pTextEdit->document()->clearUndoRedoStacks();
}

QPainterPath CGraphicsTextItem::getHighLightPath()
{
    QPainterPath path;
    path.addRect(this->rect());
    return path;
}

QTextCharFormat CGraphicsTextItem::currentCharFormat()
{
    if (m_pTextEdit != nullptr)
        return m_pTextEdit->currentFormat();

    return QTextCharFormat();
}

void CGraphicsTextItem::changToEditState(bool selectAll)
{
    if (isMutiSelected() || this->bzGroup() != nullptr)
        return;

    m_pTextEdit->setTextInteractionFlags(m_pTextEdit->textInteractionFlags() | (Qt::TextEditable));

    auto currentTool = drawScene() == nullptr ? selection : drawScene()->page()->currentTool();
    if (currentTool == selection || currentTool == text) {

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
    if (drawScene() != nullptr) {
        drawScene()->selectItem(this);
    }

    //保证自身的焦点
    toFocusEiditor();

//    //刷新鼠标样式
//    if (drawScene() != nullptr) {
//        drawScene()->refreshLook();
//    }
    //进入编辑时,清空当前的撤销重做栈
    m_pTextEdit->document()->clearUndoRedoStacks();
}

void CGraphicsTextItem::changToReadOnlyState(bool selectAll)
{
    if (m_pTextEdit == nullptr)
        return;

    if (selectAll) {
        m_pTextEdit->selectAll();
    }
    m_pProxy->setFlag(ItemHasNoContents, true);

    if (drawScene() != nullptr)
        this->drawScene()->notSelectItem(this);
}

void CGraphicsTextItem::setTextState(CGraphicsTextItem::EState state, bool selectAllText)
{
    if (textState() == state)
        return;

    if (state == EInEdit) {
        changToEditState(selectAllText);
    } else {
        changToReadOnlyState(selectAllText);
    }
}

CGraphicsTextItem::EState CGraphicsTextItem::textState() const
{
    if (m_pTextEdit == nullptr) {
        return  EReadOnly;
    }
    return (m_pProxy->flags() & ItemHasNoContents) ? EReadOnly : EInEdit;
}

//bool CGraphicsTextItem::isSelectionEmpty()
//{
//    if (m_pTextEdit != nullptr) {
//        return m_pTextEdit->textCursor().selectedText().isEmpty();
//    }
//    return true;
//}

void CGraphicsTextItem::beginPreview()
{
    qWarning() << "CGraphicsTextItem::beginPreview()------------- ";
    if (!_isPreview) {
        _isPreview = true;
        if (m_pTextEdit != nullptr) {
            QTextCursor tCur = m_pTextEdit->textCursor();
            tCur.beginEditBlock();
            m_pTextEdit->setTextCursor(tCur);
            qWarning() << "beginPreview avable undo count = " << m_pTextEdit->document()->availableUndoSteps();
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

void CGraphicsTextItem::endPreview(bool revert)
{
    //qWarning() << "CGraphicsTextItem::endPreview()------------- ";
    if (isPreview() && m_pTextEdit != nullptr) {
        QTextCursor tCur = m_pTextEdit->textCursor();
        tCur.endEditBlock();
        m_pTextEdit->setTextCursor(tCur);
        //qWarning() << "endPreview   avable undo count = " << m_pTextEdit->document()->availableUndoSteps();
    }

    if (revert) {
        if (isPreview()) {
            QTextCursor txtCursorBefore = m_pTextEdit->textCursor();
            int begin = txtCursorBefore.selectionStart();
            int end   = txtCursorBefore.selectionEnd();

            if (txtCursorBefore.hasSelection())  //如果没有选中证明没有修改过,那么就不用还原；否则则还原
                m_pTextEdit->undo();

            QTextCursor txtCursorAfter = m_pTextEdit->textCursor();
            txtCursorAfter.setPosition(begin);
            txtCursorAfter.setPosition(end, QTextCursor::KeepAnchor);
            m_pTextEdit->setTextCursor(txtCursorAfter);
            //qDebug() << "endPreview1   avable undo count = " << m_pTextEdit->document()->availableUndoSteps();
        }
    }
    _isPreview = false;
}

bool CGraphicsTextItem::isPreview()
{
    return _isPreview;
}

CTextEdit *CGraphicsTextItem::textEditor() const
{
    return m_pTextEdit;
}

//QGraphicsProxyWidget *CGraphicsTextItem::proxyWidgetItem()
//{
//    return m_pProxy;
//}

int CGraphicsTextItem::type() const
{
    return TextType;
}

void CGraphicsTextItem::setRect(const QRectF &rect)
{
    //1.修改自身的大小
    CGraphicsRectItem::setRect(rect);

    //2.修改文字编辑控件的大小(通过代理图元)
    if (m_pProxy != nullptr) {
        m_pProxy->resize(qRound(rect.width()), qRound(rect.height()));
        updateProxyItemPos();
    }

    //3.刷新形状及所处组合
    updateShapeRecursion();

    //4.更新高亮
    //if (drawScene() != nullptr && isSelected())
    //drawScene()->setHighlightHelper(mapToScene(getHighLightPath()));
}

void CGraphicsTextItem::updateProxyItemPos()
{
    const QRectF &geom = this->rect();
    m_pProxy->setPos(geom.x(), geom.y());
}

QFont CGraphicsTextItem::font() const
{
    return m_pTextEdit->currentFont();
}

void CGraphicsTextItem::setFont(const QFont &font)
{
    m_pTextEdit->setCurrentFont(font);
}

QString CGraphicsTextItem::fontStyle()
{
    return m_pTextEdit->currentFontStyle();
}

void CGraphicsTextItem::setFontStyle(const QString &style)
{
    m_pTextEdit->setCurrentFontStyle(style);
}

void CGraphicsTextItem::setFontSize(int size)
{
    m_pTextEdit->setCurrentFontSize(size);
}

int CGraphicsTextItem::fontSize()
{
    return m_pTextEdit->currentFontSize();
}

void CGraphicsTextItem::setFontFamily(const QString &family)
{
    m_pTextEdit->setCurrentFontFamily(family);
}

QString CGraphicsTextItem::fontFamily() const
{
    return m_pTextEdit->currentFontFamily();
}

void CGraphicsTextItem::updateToDefaultTextFormat()
{
    this->m_pTextEdit->selectAll();
    if (page() != nullptr) {
        setFontFamily(page()->defaultAttriVar(EFontFamily).toString());
        setFontStyle(page()->defaultAttriVar(EFontWeightStyle).toString());
        setFontSize(page()->defaultAttriVar(EFontSize).toInt());
        setTextColor(page()->defaultAttriVar(EFontColor).value<QColor>());
    }
}

void CGraphicsTextItem::loadGraphicsUnit(const CGraphicsUnit &data)
{
    SGraphicsTextUnitData *pTextData = data.data.pText;

    loadHeadData(data.head);

    if (pTextData != nullptr) {
        loadGraphicsRectUnit(pTextData->rect);

        QRectF rect(pTextData->rect.topLeft, pTextData->rect.bottomRight);

        setFont(pTextData->font);

        //手动调整过大小就意味着不需要自动根据文本调整大小了
        setAutoAdjustSize(!pTextData->manResizeFlag);

        m_pTextEdit->setHtml(pTextData->content);

        setTextColor(pTextData->color);

        m_pTextEdit->applyDefaultToFirstFormat();

        m_pTextEdit->document()->clearUndoRedoStacks();

        setRect(rect);
    }
}

void CGraphicsTextItem::setTextColor(const QColor &col)
{
//    m_pTextEdit->textCursor().beginEditBlock();
//    QTextCharFormat fmt;
//    fmt.setForeground(col);
//    setCurrentFormat(fmt);
//    m_color = col;
//    m_pTextEdit->textCursor().endEditBlock();

    m_pTextEdit->setCurrentColor(col);
}

QColor CGraphicsTextItem::textColor() const
{
    //return m_color;
    return m_pTextEdit->currentColor();
}

void CGraphicsTextItem::setCurrentFormat(const QTextCharFormat &format, bool merge)
{
    m_pTextEdit->setCurrentFormat(format, merge);
}

void CGraphicsTextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

//    //0.会有一种情况就是图元从场景删除后,Qt依旧会掉用到paint函数,这是一个BUG,
//    //a.怀疑是修改图元大小时,没有添加prepareGeometryChange()导致
//    //b也可能是代理图元proyxy的添加删除交互?
//    //当前通过该方法让其不会显示出来
//    if (scene() == nullptr) {
//        qWarning() << "----------- scene == nullptr but paint !!!! ";
//        prepareGeometryChange();
//        setRect(QRectF(0, 0, 0, 0));
//        return;
//    }

//    //1.矩形大小不正确不绘制
//    if (!rect().isValid())
//        return;

//    beginCheckIns(painter);

//    if (textState() == EReadOnly)
//        drawDocument(painter, m_pTextEdit->document(), this->rect());

//    endCheckIns(painter);

//    paintMutBoundingLine(painter, option);
    CGraphicsRectItem::paint(painter, option, widget);
}

void CGraphicsTextItem::paintSelf(QPainter *painter, const QStyleOptionGraphicsItem *option)
{
    //0.会有一种情况就是图元从场景删除后,Qt依旧会掉用到paint函数,这是一个BUG,
    //a.怀疑是修改图元大小时,没有添加prepareGeometryChange()导致
    //b也可能是代理图元proyxy的添加删除交互?
    //当前通过该方法让其不会显示出来
    if (scene() == nullptr) {
        qWarning() << "----------- scene == nullptr but paint !!!! ";
        prepareGeometryChange();
        setRect(QRectF(0, 0, 0, 0));
        return;
    }

    //1.矩形大小不正确不绘制
    if (!rect().isValid())
        return;

    beginCheckIns(painter);

    if (textState() == EReadOnly)
        drawDocument(painter, m_pTextEdit->document(), this->rect());

    endCheckIns(painter);

    paintMutBoundingLine(painter, option);
}

void CGraphicsTextItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    //changToEditState(true);
}

bool CGraphicsTextItem::isPosPenetrable(const QPointF &posLocal)
{
    Q_UNUSED(posLocal)
    return false;
}

//void CGraphicsTextItem::operatingBegin(CGraphItemEvent *event)
//{
//    if (event->toolEventType() == 3) {
//        setAutoAdjustSize(false);
//    }
//}

void CGraphicsTextItem::doScalBegin(CGraphItemScalEvent *event)
{
    Q_UNUSED(event)
    setAutoAdjustSize(false);
}

QVariant CGraphicsTextItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    return CGraphicsRectItem::itemChange(change, value);
}

void CGraphicsTextItem::drawDocument(QPainter *painter,
                                     const QTextDocument *doc,
                                     const QRectF &r)
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

bool CGraphicsTextItem::isAutoAdjustSize() const
{
    return _autoAdjustSize;
}

CGraphicsUnit CGraphicsTextItem::getGraphicsUnit(EDataReason reson) const
{
    CGraphicsUnit unit;

    unit.reson = reson;

    unit.head.dataType = this->type();
    unit.head.dataLength = sizeof(SGraphicsTextUnitData);
    unit.head.pen = this->pen();
    unit.head.brush = this->brush();
    unit.head.pos = this->pos();
    unit.head.rotate = this->drawRotation();
    unit.head.zValue = this->zValue();
    unit.head.trans = this->transform();

    unit.data.pText = new SGraphicsTextUnitData();
    unit.data.pText->rect.topLeft = this->rect().topLeft();
    unit.data.pText->rect.bottomRight = this->rect().bottomRight();
    unit.data.pText->font = /*this->m_Font*/font();
    unit.data.pText->manResizeFlag = !this->isAutoAdjustSize();
    unit.data.pText->content = this->m_pTextEdit->toHtml();
    unit.data.pText->color = /*m_color*/textColor();

    //qWarning() << "content = " << unit.data.pText->content << "font size = " << m_pTextEdit->currentFontSize();

    return  unit;
}

bool CGraphicsTextItem::isEditState() const
{
    return textState() == EInEdit;
}

void CGraphicsTextItem::toFocusEiditor()
{
    if (textState() == EInEdit) {
        //保证按键响应到textedit控件的底层(从而才能将key事件传递给textedit)
        if (curView() != nullptr) {
            curView()->setFocus();
        }
        //保证控件可编辑
        m_pTextEdit->setTextInteractionFlags(m_pTextEdit->textInteractionFlags() | (Qt::TextEditable));

        //保证自身的焦点
        m_pTextEdit->setFocus();
    }
}

void CGraphicsTextItem::doCut()
{
    m_pTextEdit->cut();
}

void CGraphicsTextItem::doCopy()
{
    m_pTextEdit->copy();
}

void CGraphicsTextItem::doPaste()
{
    qDebug() << "------------------------CGraphicsTextItem::doPaste()--------------------";
    m_pTextEdit->paste();
}

void CGraphicsTextItem::doSelectAll()
{
    m_pTextEdit->selectAll();
}

void CGraphicsTextItem::setSelectTextBlockAlign(const Qt::Alignment &align)
{
    switch (align) {
    case Qt::AlignLeft : {
        m_pTextEdit->setAlignment(Qt::AlignLeft);
        break;
    }
    case Qt::AlignRight : {
        m_pTextEdit->setAlignment(Qt::AlignRight);
        break;
    }
    case Qt::AlignHCenter : {
        m_pTextEdit->setAlignment(Qt::AlignHCenter);
        break;
    }
    case Qt::AlignTop : {
        m_pTextEdit->setAlignment(Qt::AlignTop);
        break;
    }
    case Qt::AlignBottom : {
        m_pTextEdit->setAlignment(Qt::AlignBottom);
        break;
    }
    case Qt::AlignVCenter : {
        m_pTextEdit->setAlignment(Qt::AlignVCenter);
        break;
    }
    case Qt::AlignCenter : {
        m_pTextEdit->setAlignment(Qt::AlignCenter);
        break;
    }
    }
}

void CGraphicsTextItem::doUndo()
{
    m_pTextEdit->undo();
}

void CGraphicsTextItem::doRedo()
{
    m_pTextEdit->redo();
}

void CGraphicsTextItem::doDelete()
{
    m_pTextEdit->textCursor().deleteChar();
}

void CGraphicsTextItem::setAutoAdjustSize(bool b)
{
    if (b == _autoAdjustSize)
        return;

    _autoAdjustSize = b;

    if (_autoAdjustSize && m_pTextEdit != nullptr)
        m_pTextEdit->setLineWrapMode(QTextEdit::NoWrap);
    else {
        m_pTextEdit->setLineWrapMode(QTextEdit::WidgetWidth);
    }

    //修改了是否自动调整文本大小会影响到选中节点是否显示,所以刷新一下选中图元
    if (drawScene() != nullptr) {
        drawScene()->selectGroup()->updateBoundingRect();
    }
}

