// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ctextedit.h"
#include "textitem.h"
#include "pagescene.h"
#include "itemgroup.h"
#include "pageview.h"
#include "attributemanager.h"

#include <QGraphicsProxyWidget>
#include <QMenu>
#include <QApplication>
#include <QAction>
#include <QDebug>
#include <QTextBlock>
#include <QTextEdit>
#include <malloc.h>
#include <QMimeData>
#include <QComboBox>
#include <QScrollBar>
#include <QTimer>
#include <QClipboard>

static QTextCharFormat::Property senseProerties[] = {QTextCharFormat::ForegroundBrush,
                                                     QTextCharFormat::FontFamily,
                                                     QTextCharFormat::FontPointSize,
                                                     QTextCharFormat::FontWeight
                                                    };
static int senseProertiesCount = sizeof(senseProerties) / sizeof(QTextCharFormat::ForegroundBrush);




class TextEdit::TextEdit_private
{
public:
    explicit TextEdit_private(TextEdit *qq, TextItem *it): q(qq), m_pItem(it) {}

    void initTextContextMenu()
    {
        m_textMenu = new QMenu(m_pItem->pageView());

        m_textCutAction = new QAction(tr("Cut"), m_textMenu);
        m_textCopyAction = new QAction(tr("Copy"), m_textMenu);
        m_textPasteAction = new QAction(tr("Paste"), m_textMenu);
        m_textSelectAllAction = new QAction(tr("Select All"), m_textMenu);
        m_textUndoAct = new QAction(tr("Undo"), m_textMenu);
        m_textRedoAct = new QAction(tr("Redo"), m_textMenu);
        m_textLeftAlignAct = new QAction(tr("Text Align Left"), m_textMenu);                 // 左对齐
        m_textRightAlignAct = new QAction(tr("Text Align Right"), m_textMenu);            // 右对齐
        m_textCenterAlignAct = new QAction(tr("Text Align Center"), m_textMenu);     //  水平垂直居中对齐
        m_textDeleteAction = new QAction(tr("Delete"), m_textMenu);

        m_textMenu->addAction(m_textCutAction);
        m_textMenu->addAction(m_textCopyAction);
        m_textMenu->addAction(m_textPasteAction);
        m_textMenu->addAction(m_textSelectAllAction);
        m_textMenu->addAction(m_textDeleteAction);
        m_textMenu->addSeparator();
        m_textMenu->addAction(m_textUndoAct);
        m_textMenu->addAction(m_textRedoAct);
        m_textMenu->addSeparator();

        m_textMenu->addAction(m_textLeftAlignAct);
        m_textMenu->addAction(m_textRightAlignAct);
        m_textMenu->addAction(m_textCenterAlignAct);
    }

    void initTextContextMenuConnection()
    {
        q->connect(m_textCutAction, &QAction::triggered, q, &TextEdit::cut);
        q->connect(m_textCopyAction, &QAction::triggered, q, &TextEdit::copy);
        q->connect(m_textPasteAction, &QAction::triggered, q, &TextEdit::paste);
        q->connect(m_textSelectAllAction, &QAction::triggered, q, &TextEdit::selectAll);
        q->connect(m_textUndoAct, &QAction::triggered, q, &TextEdit::undo);
        q->connect(m_textRedoAct, &QAction::triggered, q, &TextEdit::redo);
        q->connect(m_textDeleteAction, &QAction::triggered, q, [ = ]() {
            q->textCursor().deleteChar();
        });

        // 设置文字图元内部对齐方式
        connect(m_textLeftAlignAct, &QAction::triggered, q, [ = ]() {
            q->setAlignment(Qt::AlignLeft);
        });
        connect(m_textRightAlignAct, &QAction::triggered, q, [ = ]() {
            q->setAlignment(Qt::AlignRight);
        });
        connect(m_textCenterAlignAct, &QAction::triggered, q, [ = ]() {
            q->setAlignment(Qt::AlignCenter);
        });
    }

    void setTextAlignMenuActionStatus()
    {
        menu();
        // 根据是否是两点情况显示对齐
        if (m_pItem->isWrap()) {
            m_textLeftAlignAct->setEnabled(false);
            m_textRightAlignAct->setEnabled(false);
            m_textCenterAlignAct->setEnabled(false);
        } else {
            m_textLeftAlignAct->setEnabled(true);
            m_textRightAlignAct->setEnabled(true);
            m_textCenterAlignAct->setEnabled(true);
        }

        bool selectionIsEmpty = q->textCursor().selectedText().isEmpty();
        m_textCutAction->setEnabled(!selectionIsEmpty);
        m_textCopyAction->setEnabled(!selectionIsEmpty);
        m_textDeleteAction->setEnabled(!selectionIsEmpty);
        m_textPasteAction->setEnabled(q->canPaste());
        m_textSelectAllAction->setEnabled(true);
        m_textUndoAct->setEnabled(q->document()->isUndoAvailable());
        m_textRedoAct->setEnabled(q->document()->isRedoAvailable());
    }

    QMenu *menu()
    {
        if (m_textMenu == nullptr) {
            initTextContextMenu();
            initTextContextMenuConnection();
        }
        return m_textMenu;
    }

    TextItem *m_pItem = nullptr;
    QTextCharFormat _selectionFmt;

    QSet<QTextCharFormat::Property> _blockedProperties;

    bool _sflag = false;
    //记录文本块字符数
    int document_Charaters;

    QTextCharFormat _defaultFormat;
    QTimer *_updateTimer = nullptr;

    //文字图元右键菜单
    QMenu *m_textMenu = nullptr;              //文字菜单
    QAction *m_textCutAction = nullptr;       //文字剪切
    QAction *m_textCopyAction = nullptr;      //文字复制
    QAction *m_textPasteAction = nullptr;     //文字粘贴
    QAction *m_textSelectAllAction = nullptr; //文字全选
    QAction *m_textDeleteAction = nullptr;    //文字删除
    QAction *m_textUndoAct = nullptr;         //文字撤销
    QAction *m_textRedoAct = nullptr;         //文字重做
    QAction *m_textLeftAlignAct = nullptr;    //文字左对齐
    QAction *m_textRightAlignAct = nullptr;   //文字右对齐
    QAction *m_textCenterAlignAct = nullptr;  //文字水平垂直居中对齐(目前Qt只支持水平方向的对齐)

    TextEdit *q;
};

TextEdit::TextEdit(TextItem *item, QWidget *parent)
    : QTextEdit(parent), TextEdit_d(new TextEdit_private(this, item))
{
    //初始化字体
    connect(this, &TextEdit::textChanged, this, &TextEdit::markCursorDataDirty);
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(onCursorPositionChanged()));
    connect(this, &TextEdit::selectionChanged, this, &TextEdit::onSelectionChanged);
    connect(this, &TextEdit::currentCharFormatChanged, this, &TextEdit::onCurrentCharFormatChanged);

    this->setLineWrapMode(NoWrap);
    this->setFrameStyle(NoFrame);
    this->setLineWidth(0);

    updateBgColorTo(QColor(255, 255, 255), true);

    this->setMinimumSize(QSize(1, 1));
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setFrameShape(QTextEdit::NoFrame);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //初始化文本块行数
    d_TextEdit()->document_Charaters = document()->characterCount();

//    QTextCharFormat fmt;
//    QFontDatabase base;
//    fmt.setFontFamily(base.families().first());
//    this->setDefaultFormat(fmt);
}

TextEdit::~TextEdit()
{
    d_TextEdit()->m_pItem = nullptr;
}

QTextCharFormat TextEdit::currentFormat(bool considerSelection)
{
    if (considerSelection) {
        if (this->textCursor().hasSelection()) {
            updateSelectionFormat();
            return d_TextEdit()->_selectionFmt;
        }
    }
    return currentCharFormat();
}

void TextEdit::setCurrentFormat(const QTextCharFormat &format, bool merge)
{
    //不设置会有清空所有文本后文字格式错误问题
    if (textCursor().hasSelection()) {
        //同时设置默认的块的字体格式
        textCursor().mergeBlockCharFormat(format);
    }
    merge ? mergeCurrentCharFormat(format) : setCurrentCharFormat(format);
}

QColor TextEdit::currentColor()
{
    auto fmt = currentFormat();
    if (!fmt.hasProperty(QTextCharFormat::ForegroundBrush)) {
        return QColor();
    }
    return fmt.foreground().color();
}

void TextEdit::setCurrentColor(const QColor &color)
{
    QTextCharFormat fmt;
    fmt.setForeground(color);
    setCurrentFormat(fmt, true);
}

QFont TextEdit::currentFont(bool considerSelection)
{
    return currentFormat(considerSelection).font();
}

void TextEdit::setCurrentFont(const QFont &ft)
{
    QTextCharFormat fmt;
    fmt.setFont(ft);
    setCurrentFormat(fmt, true);
}

int TextEdit::currentFontSize()
{
    return qRound(currentFormat().fontPointSize());
}

void TextEdit::setCurrentFontSize(const int sz)
{
    QTextCharFormat fmt;
    fmt.setFontPointSize(sz);
    setCurrentFormat(fmt, true);
}

QString TextEdit::currentFontFamily()
{
    return currentFormat().fontFamily();
}

void TextEdit::setCurrentFontFamily(const QString &family)
{
    //qDebug() << "CTextEdit::setCurrentFontFamily = " << family;
    QTextCharFormat fmt;
    fmt.setFontFamily(family);
    setCurrentFormat(fmt, true);
}

QString TextEdit::currentFontStyle()
{
    auto fmt = currentFormat();
    int w = fmt.hasProperty(QTextCharFormat::FontWeight) ? fmt.intProperty(QTextCharFormat::FontWeight) : -1;
    return toStyle(w);
}

void TextEdit::setCurrentFontStyle(const QString &style)
{
    QTextCharFormat fmt;
    int wt = toWeight(style);
    fmt.setFontWeight(wt);
    setCurrentFormat(fmt, true);
}

void TextEdit::onTextChanged()
{
    if (d_TextEdit()->m_pItem == nullptr)
        return;

    // 如果是两点的状态高度需要自适应
    if (d_TextEdit()->m_pItem->isWrap()) {
        QSizeF size = this->document()->size();
        QRectF rect = d_TextEdit()->m_pItem->orgRect();
        rect.setHeight(size.height());
        rect.setWidth(size.width());
        d_TextEdit()->m_pItem->setRect(rect);
    } else {
        //自动换行时,如果当前的文字字体太大那么输入的文字可能显示不到,我们就设置
        int ftHeight = QFontMetrics(this->currentFont(false)).lineSpacing();
        if (this->height() < ftHeight) {
            auto curRect = d_TextEdit()->m_pItem->orgRect();
            curRect.setHeight(ftHeight);
            QMetaObject::invokeMethod(this, [ = ]() {
                d_TextEdit()->m_pItem->setRect(curRect);
            }, Qt::QueuedConnection);
        }
    }

}

void TextEdit::onCursorPositionChanged()
{
    if (!this->textCursor().hasSelection())
        updatePropertyWidget();
}

void TextEdit::onSelectionChanged()
{
    static int s_preSelectionBeginPos = -1;
    static int s_preSelectionEndPos = -1;

    int newBegin = textCursor().selectionStart();
    int newEnd = textCursor().selectionEnd();
    if (s_preSelectionBeginPos !=  newBegin ||
            s_preSelectionEndPos != newEnd) {
        s_preSelectionBeginPos = newBegin;
        s_preSelectionEndPos = newEnd;
        updatePropertyWidget();
    }
}

void TextEdit::onCurrentCharFormatChanged(const QTextCharFormat &format)
{
    Q_UNUSED(format)

    if (!textCursor().hasSelection())  //使用控件自身刷新
        updatePropertyWidget();
}

void TextEdit::updatePropertyWidget()
{
    if (d_TextEdit()->_updateTimer == nullptr) {
        d_TextEdit()->_updateTimer = new QTimer(this);
        d_TextEdit()->_updateTimer->setSingleShot(true);
        connect(d_TextEdit()->_updateTimer, &QTimer::timeout, this, [ = ]() {
            // 刷新属性
            if (d_TextEdit()->m_pItem->page() != nullptr) {
                d_TextEdit()->m_pItem->page()->showAttributions(d_TextEdit()->m_pItem->attributions(),
                                                                AttributionManager::ItemAttriChanged);
            }
        });
    }
    d_TextEdit()->_updateTimer->start(70);
}

void TextEdit::applyDefaultToFirstFormat()
{
    if (textCursor().selectionStart() != textCursor().selectionEnd()) {
        return;
    }

    //BUG,if the text be empty, then the format will unexpected be clear,so there do that to
    setDefaultFormat(firstPosFormat());
}

void TextEdit::insertFromMimeData(const QMimeData *source)
{
    if (source && source->hasText()) {
        this->insertPlainText(source->text());
        return;
    }
    QTextEdit::insertFromMimeData(source);
}

void TextEdit::contextMenuEvent(QContextMenuEvent *e)
{
    //qWarning() << "CTextEdit::contextMenuEvent ------- " << e;
    //屏蔽自带的菜单
    e->accept();
    setMenu();
}

void TextEdit::inputMethodEvent(QInputMethodEvent *e)
{
    if (!e->commitString().isEmpty()) {
        QTextEdit::inputMethodEvent(e);
    }
}

void TextEdit::focusOutEvent(QFocusEvent *e)
{
    //焦点离开取消选中
    clearSelectState();
    QTextEdit::focusOutEvent(e);
}

void TextEdit::mouseDoubleClickEvent(QMouseEvent *event)
{
    QTextEdit::mouseDoubleClickEvent(event);
}

bool TextEdit::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyRelease) {
        QKeyEvent *keyEvnt = static_cast<QKeyEvent *>(event);
        //1.重载实现自定义的撤销还原快捷键
        if (keyEvnt->modifiers() == Qt::CTRL) {
            if (keyEvnt->key() == Qt::Key_Y) {
                redo();
                event->accept();
                return true;
            } else if (keyEvnt->key() == Qt::Key_Z) {
                undo();
                event->accept();
                return true;
            }
        }

        //2.屏蔽原来默认的重做快捷键
        if ((keyEvnt->modifiers() & Qt::CTRL) && (keyEvnt->modifiers()&Qt::SHIFT) && keyEvnt->key() == Qt::Key_Z) {
            event->accept();
            return true;
        }
    }

    return QTextEdit::eventFilter(obj, event);
}

QVector<QTextLayout::FormatRange> TextEdit::getCharFormats(int posBegin, int posEnd)
{
    QVector<QTextLayout::FormatRange> fmts;
    int beginPos = posBegin;
    int endPos = posEnd;
    auto beginBlk = document()->findBlock(beginPos);
    auto endBlk   = document()->findBlock(endPos).next();
    for (auto itBk = beginBlk; itBk != endBlk; itBk = itBk.next()) {
        auto fmtVector = itBk.textFormats();
        foreach (auto fmt, fmtVector) {
            int spanBeginPosInDoc = itBk.position() + fmt.start;
            int spanEndPosInDoc  = spanBeginPosInDoc + fmt.length - 1;

            if (spanEndPosInDoc < beginPos || spanBeginPosInDoc > endPos) {
                continue;
            }
            //qDebug() << "s b = " << beginPos << "e b = " << endPos << "span b = " << spanBeginPosInDoc << "span e = " << spanEndPosInDoc;
            int newStart = qMax(spanBeginPosInDoc, beginPos);
            int newEnd   = qMin(spanEndPosInDoc, endPos);
            fmt.start  = newStart;
            fmt.length = newEnd - newStart + 1;

            fmts.append(fmt);
        }
    }
    //qDebug() << "fmts count ======= " << fmts.count();
    return fmts;
}

void TextEdit::updateSelectionFormat()
{
    QTextCursor c = textCursor();
    int beginPos = qMin(c.selectionStart(), c.selectionEnd());
    int endPos = qMax(c.selectionStart(), c.selectionEnd()) - 1;  //光标的首位(pos == 0)位于第一个字符之前,所以其结尾位置需要-1以和字符在doc中的索引保持同步
    auto fmts = getCharFormats(beginPos, endPos);

    //格式数据是否有冲突
    QTextCharFormat fmt = fmts.first().format;
    for (int i = 1; i < fmts.count(); ++i) {
        QTextCharFormat ft = fmts.at(i).format;
        for (int j = 0; j < senseProertiesCount; ++j) {
            QTextCharFormat::Property property = senseProerties[j];
            if (fmt.hasProperty(property)) {
                if (fmt.property(property) != ft.property(property)) {
                    fmt.clearProperty(property);
                }
            }
        }
    }
    d_TextEdit()->_selectionFmt = fmt;
}

int TextEdit::toWeight(const QString &styleName)
{
    int weight = 0;
    if (styleName == "Thin") {
        weight = 0;
    } else if (styleName == "ExtraLight") {
        weight = 12;
    } else if (styleName == "Light") {
        weight = 25;
    } else if (styleName == "Normal" || styleName == "Regular") {
        weight = 50;
    } else if (styleName == "Medium") {
        weight = 57;
    } else if (styleName == "DemiBold") {
        weight = 63;
    } else if (styleName == "Bold") {
        weight = 75;
    } else if (styleName == "ExtraBold") {
        weight = 81;
    } else if (styleName == "Black") {
        weight = 87;
    }
    return weight;
}

QString TextEdit::toStyle(const int &weight)
{
    switch (weight) {
    case 0: return "Thin";
    case 12: return "ExtraLight";
    case 25: return "Light";
    case 50: return "Regular";
    case 57: return "Medium";
    case 63: return "DemiBold";
    case 75: return "Bold";
    case 81: return "ExtraBold";
    case 87: return "Black";
    default: return "";
    }
}

QTextCharFormat TextEdit::firstPosFormat() const
{
    if (!document()->isEmpty()) {
        return document()->firstBlock().textFormats().first().format;
    }
    return currentCharFormat();
}

void TextEdit::setDefaultFormat(const QTextCharFormat &format)
{
    d_TextEdit()->_defaultFormat = format;
    QSignalBlocker bloker(this);
    QTextCharFormat fmt = d_TextEdit()->_defaultFormat;
    setCurrentFormat(fmt);
    textCursor().mergeBlockCharFormat(fmt);
    this->setAlignment(this->alignment());
    textCursor().setPosition(0);
}

void TextEdit::markCursorDataDirty()
{
    // 取得当前的光标选取索引
    QTextCursor cursor = textCursor();

    int current_Count = document()->characterCount();

    //没有选中文字大小，并且没有输入不做修改
    if (cursor.selectionStart() == cursor.selectionEnd() && current_Count == d_TextEdit()->document_Charaters) {
        return;
    }

    //记录文本块字符数
    d_TextEdit()->document_Charaters = current_Count;
    // 提醒文本重新布局
    document()->markContentsDirty(cursor.selectionStart(), cursor.selectionEnd() - cursor.selectionStart());
    // 文本变更, 根据新的布局自适应大小
    onTextChanged();
}

void TextEdit::setMenu()
{
    if (d_TextEdit()->m_pItem != nullptr) {
        d_TextEdit()->setTextAlignMenuActionStatus();
        d_TextEdit()->menu()->popup(QCursor::pos());
    }
}

void TextEdit::clearSelectState()
{
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    setTextCursor(cursor);
}

void TextEdit::updateBgColorTo(const QColor c, bool laterDo)
{
    QPalette palette(this->palette());
    palette.setBrush(QPalette::Base, c);
    if (laterDo) {
        QMetaObject::invokeMethod(this, [ = ]() {
            this->setPalette(palette);
        }, Qt::QueuedConnection);
    } else {
        this->setPalette(palette);
    }
}
