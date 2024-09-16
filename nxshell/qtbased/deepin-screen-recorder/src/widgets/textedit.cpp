// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textedit.h"
#include "../utils/configsettings.h"
#include "../utils/baseutils.h"
#include "../utils.h"
#include <DPalette>

#include <QDebug>
#include <QPen>
#include <QScrollBar>
#include <QPixmap>
#include <QFontMetricsF>
#include <QApplication>


DGUI_USE_NAMESPACE

//const QSize CURSOR_SIZE = QSize(5, 20);
const int TEXT_MARGIN = 12;

TextEdit::TextEdit(int index, DWidget *parent)
    : DPlainTextEdit(parent),
      m_textColor(Qt::red)
{
    m_index = index;
    setLineWrapMode(DPlainTextEdit::NoWrap); //不自动换行
    setContextMenuPolicy(Qt::NoContextMenu);//不需要菜单
    int defaultColorIndex = ConfigSettings::instance()->getValue("text", "color_index").toInt();
    QColor defaultColor = BaseUtils::colorIndexOf(defaultColorIndex);
    setColor(defaultColor);
    QFont textFont;
    //默认字体大小
    int defaultFontSize = ConfigSettings::instance()->getValue("text", "fontsize").toInt();
    textFont.setPixelSize(defaultFontSize);
    //设置默认字体
    this->document()->setDefaultFont(textFont);
//    this->setLineWrapMode(QPlainTextEdit::WidgetWidth);

    //设置光标
    QTextCursor cursor = textCursor();
    QTextBlockFormat textBlockFormat = cursor.blockFormat();
    textBlockFormat.setAlignment(Qt::AlignLeft);
    cursor.mergeBlockFormat(textBlockFormat);

    QFontMetricsF m_fontMetric = QFontMetricsF(this->document()->defaultFont());
    QSizeF originSize = QSizeF(m_fontMetric.boundingRect("d").width()  + TEXT_MARGIN,  m_fontMetric.boundingRect("d").height());
    //记录当前光标的高度
    m_cursorHeight = cursorRect().height();
    qDebug() << "m_cursorHeight: "  << m_cursorHeight;

    this->resize(static_cast<int>(originSize.width()), static_cast<int>(originSize.height()));
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    //这里有两个信号容易搞混，contentsChanged这个是更改之后才会发信号，contentsChange这个信号更改之前发信号
    connect(this->document(), &QTextDocument::contentsChanged, this,  [ = ] {
        updateContentSize(this->toPlainText());
    });
    m_currentCursor = QCursor().pos();
}

int TextEdit::getIndex()
{
    return m_index;
}

void TextEdit::setColor(QColor c)
{
    qDebug() << "set color";
    m_textColor = c;
//    setStyleSheet(QString("TextEdit {background-color:  transparent;"
//                          " color: %1; border: none;}").arg(m_textColor.name()));

    DPalette pa = this->palette();
    pa.setColor(DPalette::Base, QColor(255, 255, 255, 0));
    pa.setColor(DPalette::Dark, QColor(255, 255, 255, 0));
    pa.setColor(DPalette::Light, QColor(255, 255, 255, 0));
    pa.setColor(DPalette::Text, c);
    this->setPalette(pa);
    this->updateGeometry();
}
/*
 * never used
QColor TextEdit::getColor()
{
    return m_textColor;
}
*/
void TextEdit::setFontSize(int fontsize)
{
    QFont font;
    font.setPixelSize(fontsize);
    this->document()->setDefaultFont(font);
    this->updateGeometry();
    m_cursorHeight = cursorRect().height();

    updateContentSize(this->toPlainText());
}

void TextEdit::inputMethodEvent(QInputMethodEvent *e)
{
    DPlainTextEdit::inputMethodEvent(e);

    QString virtualStr = this->toPlainText() + e->preeditString();
    updateContentSize(virtualStr);
}

void TextEdit::updateContentSize(QString content)
{
//    qDebug() << "=============" << m_count << "===============";
//    qDebug() << "content: " << content << "content.size(): " << content.size();
//    qDebug() << "blockCount() : " << blockCount() ;
//    qDebug() << "this->document()->toPlainText(): " << this->document()->toPlainText() ;
//    qDebug() << "this->document()->size(): " << this->document()->size() ;
//    qDebug() << "this->document()->textWidth(): " << this->document()->textWidth();
//    qDebug() << "this->document()->idealWidth(): " << this->document()->idealWidth();
//    qDebug() << "this->document()->indentWidth(): " << this->document()->indentWidth();
//    qDebug() << "this->document()->lineCount(): " << this->document()->lineCount();
//    qDebug() << "this->document()->defaultFont().pixelSize(): " << this->document()->defaultFont().pixelSize();
    QFontMetricsF fontMetric = QFontMetricsF(this->document()->defaultFont());
    //当前文本的大小--当前选中的文本
    QSizeF docSize =  fontMetric.size(Qt::TextDontClip,  content);
//    qDebug() << "docSize: " << docSize ;
//    qDebug() << "docSize + TEXT_MARGIN: " << (docSize.width() + TEXT_MARGIN);
    qreal mwidth = 0;
    mwidth = this->document()->size().width() + 2;

//    qDebug() << "mwidth: " << mwidth ;
    //根据当前光标的高度及行数，调整当前编辑框的大小
    this->setMinimumSize(static_cast<int>(mwidth), static_cast<int>(m_cursorHeight *  blockCount() + TEXT_MARGIN));
    this->resize(static_cast<int>(mwidth), static_cast<int>(m_cursorHeight *  blockCount() + TEXT_MARGIN));
    emit  repaintTextRect(this,  QRectF(this->x(), this->y(),
                                        mwidth, m_cursorHeight *  blockCount() + TEXT_MARGIN));

//    qDebug() << "this->size(): " << this->size();
//    qDebug() << "=============" << m_count << "===============";
//    m_count++;
}

void TextEdit::setEditing(bool edit)
{
    m_editing = edit;
}

void TextEdit::setSelecting(bool select)
{
    if (select) {
        emit textEditSelected(getIndex());
    }

//    emit clickToEditing(getIndex());
}

void TextEdit::updateCursor()
{
//    setTextColor(Qt::green);
}

void TextEdit::setCursorVisible(bool visible)
{
    if (visible) {
        setCursorWidth(1);
    } else {
        setCursorWidth(0);
    }
}
void TextEdit::mousePressEvent(QMouseEvent *e)
{
    qDebug() << "TextEdit mousePressEvent" << e->pos();
//    if (!this->isReadOnly()) {
//        DPlainTextEdit::mousePressEvent(e);
//        return;
//    }
    m_currentCursor = QCursor().pos();
    if (m_editing == true) {
        DPlainTextEdit::mousePressEvent(e);
        return;
    }

    if (e->button() == Qt::LeftButton) {
        m_isPressed = true;
        m_pressPoint = QPointF(mapToGlobal(e->pos()));

        if (this->isReadOnly()) {
            qDebug() << "text select:" << getIndex();
            emit textEditSelected(getIndex());
        }
    }

    DPlainTextEdit::mousePressEvent(e);
}
/*
void TextEdit::wheelEvent(QWheelEvent *e)
{
    Q_UNUSED(e);
    return;
}
*/
void TextEdit::mouseMoveEvent(QMouseEvent *e)
{
    m_currentCursor = QCursor().pos();
//    if (!(e->x() > 0 && e->x() < this->geometry().width() && e->y() > 0 && e->y() < this->geometry().height())) {
//        return;
//    }
    if (m_editing == true) {
        DPlainTextEdit::mouseMoveEvent(e);
        return;
    }
    qApp->setOverrideCursor(Qt::ClosedHandCursor);
    QPointF posOrigin = QPointF(mapToGlobal(e->pos()));
    QPointF movePos = QPointF(posOrigin.x(), posOrigin.y());

    if (m_isPressed && movePos != m_pressPoint) {
        this->move(static_cast<int>(this->x() + movePos.x() - m_pressPoint.x()),
                   static_cast<int>(this->y() + movePos.y() - m_pressPoint.y()));

        emit  repaintTextRect(this,  QRectF(qreal(this->x()), qreal(this->y()),
                                            this->width(),  this->height()));
        m_pressPoint = movePos;
    }


    DPlainTextEdit::mouseMoveEvent(e);
}

void TextEdit::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_editing == true) {
        DPlainTextEdit::mouseReleaseEvent(e);
        return;
    }
    m_isPressed = false;
    if (this->isReadOnly()) {
        setMouseTracking(false);
        DPlainTextEdit::mouseReleaseEvent(e);
        return;
    }


}

void TextEdit::mouseDoubleClickEvent(QMouseEvent *e)
{
    this->setReadOnly(false);
    this->setCursorVisible(true);
//    emit backToEditing();
    emit clickToEditing(getIndex());

    DPlainTextEdit::mouseDoubleClickEvent(e);
}

void TextEdit::keyPressEvent(QKeyEvent *e)
{
    DPlainTextEdit::keyPressEvent(e);
    if (m_isPressed) {
        qDebug() << "通过键盘移动光标";
        Utils::cursorMove(m_currentCursor, e);
    }
    if (e->key() == Qt::Key_Escape && !this->isReadOnly()) {
        this->setReadOnly(true);
    }
}

void TextEdit::focusInEvent(QFocusEvent *e)
{
    setAttribute(Qt::WA_InputMethodEnabled);
    DPlainTextEdit::focusInEvent(e);
}

//void TextEdit::focusOutEvent(QFocusEvent *e)
//{
//    emit textEditFinish(getIndex());
//    setReadOnly(true);
//    releaseKeyboard();
//    setEditing(false);
//    DPlainTextEdit::focusOutEvent(e);
//}

TextEdit::~TextEdit() {}
