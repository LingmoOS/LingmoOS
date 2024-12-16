// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "memorywidget.h"

#include <QDebug>
#include <QFont>
#include <QModelIndex>
#include <QApplication>
#include <QPainter>
#include <QPalette>
#include <QEvent>
#include <QPen>
#include <QContextMenuEvent>
#include <QMenu>
#include <QClipboard>
#include <DGuiApplicationHelper>

#include "../utils.h"
#include "../../3rdparty/core/settings.h"
#include "../memorypublic.h"
#include "../globaldefine.h"

const int STANDARD_MWIDGET_HEIGHT = 260; //标准模式memorywidget高度
const int SCIENTIFIC_MWIDGET_HEIGHT = 302; //科学模式memorywidget高度
const int PROGRAMMER_MWIDGET_HEIGHT = 230; //程序猿模式memorywidget高度
const int STANDARD_ITEM_WIDTH = 344; //标准模式宽度
const int PRO_SCI_ITEM_WIDTH = 451; //科学-程序猿模式最小宽度
const int STANDARD_FORMAT_PREC = 15; //标准模式科学计数位数
const int SCIENTIFIC_FORMAT_PREC = 31; //科学模式科学计数位数
const int MAXSIZE = 500; //内存保存最大数
//无内存时item高度
#define NOMEMORYHEIGHT(mode,labelheight)\
    {\
        if(mode != 1){\
            labelheight = m_listwidget->frameRect().height() + 30;\
        }\
        else {\
            labelheight = 302;\
        }\
    }

MemoryWidget::MemoryWidget(int mode, QWidget *parent)
    : QWidget(parent)
    , m_listwidget(new MemoryListWidget(this))
    , m_clearbutton(new IconButton(this, 1))
    , m_memoryDelegate(new MemoryItemDelegate(this))
{
    this->setAttribute(Qt::WA_DeleteOnClose);
    m_label = new QLabel(this);
    m_calculatormode = mode;
    m_evaluator = Evaluator::instance();
    this->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *lay = new QVBoxLayout(this);
    QHBoxLayout *layH = new QHBoxLayout();

    lay->setSpacing(0);
    lay->setMargin(0);
    lay->addWidget(m_listwidget);

    m_listwidget->setFrameShape(QFrame::NoFrame); //设置边框类型，无边框
    switch (mode) {
    case 0:
        m_listwidget->setFixedHeight(STANDARD_MWIDGET_HEIGHT);
        m_itemwidth = STANDARD_ITEM_WIDTH;
        m_precision = STANDARD_FORMAT_PREC;
        break;
    case 1:
        m_listwidget->setFixedHeight(SCIENTIFIC_MWIDGET_HEIGHT);
        m_itemwidth = PRO_SCI_ITEM_WIDTH;
        m_precision = SCIENTIFIC_FORMAT_PREC;
        break;
    default:
        m_listwidget->setFixedHeight(PROGRAMMER_MWIDGET_HEIGHT);
        m_itemwidth = PRO_SCI_ITEM_WIDTH;
        m_precision = STANDARD_FORMAT_PREC;
        break;
    }
    m_listwidget->setVerticalScrollMode(QListView::ScrollPerPixel); //鼠标滚轮滚动一次一个像素
    m_listwidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn); //设置垂直滚条
    m_listwidget->setAutoScroll(false); //鼠标在视口边缘时是否自动滚动内容
    m_listwidget->setSelectionRectVisible(false); //选择矩形框是否可见
    m_listwidget->setItemDelegate(m_memoryDelegate);
    m_label->hide();
    memoryclean();
    lay->addStretch();
    layH->addStretch();

    layH->addWidget(m_clearbutton);
    layH->addSpacing(10);
    if (mode == 1)
        m_clearbutton->hide();

    lay->addLayout(layH);
    lay->addSpacing(6);
    this->setLayout(lay);

    m_label->setText(tr("Nothing saved in memory"));
    m_label->setAlignment(Qt::AlignCenter); //label字体居右，居上
    int labelheight = 0;
    NOMEMORYHEIGHT(m_calculatormode, labelheight);
    m_label->setFixedSize(QSize(m_itemwidth, labelheight));
    m_label->setAttribute(Qt::WA_TranslucentBackground); //label窗体透明
    QFont font;
    font.setPixelSize(16);
    m_label->setFont(font);
    m_label->setAttribute(Qt::WA_TransparentForMouseEvents, true); //label鼠标穿透

    m_listwidget->installEventFilter(this);
    m_clearbutton->installEventFilter(this);
    this->installEventFilter(this);
    if (m_calculatormode != 1)
        setTabOrder(m_listwidget, m_clearbutton);
}

/**
 * @brief MemoryWidget::generateData
 * 向数字内存中增加一个数字，同时向保存Quantity的list中添加数据，一一对应
 * @param answer
 * 将输入的数字转换为quantity类型
 */
void MemoryWidget::generateData(const Quantity answer, bool ismax)
{
    m_label->hide();
    if (ismax)
        m_listwidget->takeItem(MAXSIZE - 1);
    if (m_memorypublic->isEmpty()) {
        m_listwidget->clear();
    }
    QListWidgetItem *item1 = new QListWidgetItem();
    item1->setFlags(Qt::ItemIsEditable);

    QFont font;
    font.setPixelSize(30);
    item1->setFont(font);
    item1->setSizeHint(QSize(m_itemwidth, 40 + 45 * m_line));
    MemoryItemWidget *widget = new MemoryItemWidget(this);
    widget->setFixedSize(QSize(m_itemwidth, 40 + 45 * m_line));

    m_listwidget->insertItem(0, item1);
    m_listwidget->setItemWidget(item1, widget);

    QString formatResult = QString();
    if (m_calculatormode == 2) {
        const QString result = programmerResult(answer);
        formatResult = Utils::formatThousandsSeparatorsPro(result, Settings::instance()->programmerBase);
    } else {
        const QString result = DMath::format(answer, Quantity::Format::General() + Quantity::Format::Precision(m_precision));
        formatResult = Utils::formatThousandsSeparators(result);
    }
    formatResult = setitemwordwrap(formatResult);
    widget->setTextLabel(formatResult);

    connect(widget, &MemoryItemWidget::plusbtnclicked, this, [ = ]() {
        int row = m_listwidget->row(item1);
        emit MemoryWidget::widgetplus(row);
    });
    connect(widget, &MemoryItemWidget::minusbtnclicked, this, [ = ]() {
        int row = m_listwidget->row(item1);
        emit MemoryWidget::widgetminus(row);
    });
    connect(widget, &MemoryItemWidget::cleanbtnclicked, this, [ = ]() {
        emit widgetclean(m_listwidget->row(item1), m_calculatormode, false);
        m_listwidget->takeItem(m_listwidget->row(item1));
        delete item1;
        if (m_listwidget->count() == 0) {
            m_memorypublic->memoryclean();
        }
    });
    widget->themetypechanged(m_themetype);
    connect(this, &MemoryWidget::themechange, widget, &MemoryItemWidget::themetypechanged);
    connect(widget, &MemoryItemWidget::menuclean, this, [ = ]() { //item菜单MC
        emit widgetclean(m_listwidget->row(item1), m_calculatormode, true);
        m_listwidget->takeItem(m_listwidget->row(item1));
        delete item1;
        if (m_listwidget->count() == 0) {
            m_memorypublic->memoryclean();
        }
    });
    connect(widget, &MemoryItemWidget::menucopy, this, [ = ]() { //item菜单复制
        QClipboard *clipboard = QApplication::clipboard();
        MemoryItemWidget *w1 = static_cast<MemoryItemWidget *>(m_listwidget->itemWidget(item1));
        clipboard->setText(w1->textLabel().remove("\n"));
    });
    connect(widget, &MemoryItemWidget::menuplus, this, [ = ]() { //item菜单M+
        int row = m_listwidget->row(item1);
        emit MemoryWidget::widgetplus(row);
    });
    connect(widget, &MemoryItemWidget::menuminus, this, [ = ]() { //item菜单M-
        int row = m_listwidget->row(item1);
        emit MemoryWidget::widgetminus(row);
    });
}

void MemoryWidget::mousePressEvent(QMouseEvent *event)
{
    QMouseEvent *pEvent = static_cast<QMouseEvent *>(event);
    m_mousepoint = pEvent->pos();

    QRect rect(this->frameGeometry());
    if (rect.contains(m_mousepoint) == true)
        emit insidewidget();
    QWidget::mousePressEvent(event);
}

/**
 * @brief 利用事件过滤器防止打开内存界面时focus到titlebar上
 */
bool MemoryWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == this && event->type() == QEvent::KeyPress) {
        QKeyEvent *key_event = static_cast < QKeyEvent *>(event); //将事件转化为键盘事件
        if (key_event->key() == Qt::Key_Tab) {
            if (!m_memorypublic->isEmpty()) {
                m_listwidget->setCurrentRow(m_currentrow);
                m_listwidget->setFocus();
            } else {
                setFocus();
            }
            return true;
        }
    }
    if (m_calculatormode != 1) {
        if (event->type() == QEvent::FocusOut && !(m_listwidget->hasFocus() || m_clearbutton->hasFocus() || this->hasFocus())) {
            emit hideWidget();
        }
        if (obj == m_listwidget || obj == m_clearbutton) {
            if (event->type() == QEvent::KeyPress) {
                QKeyEvent *key_event = static_cast < QKeyEvent *>(event); //将事件转化为键盘事件
                if (key_event->key() == Qt::Key_Tab) {
                    if (m_listwidget->hasFocus()) {
                        m_clearbutton->setFocus(Qt::TabFocusReason);
                    } else if (m_clearbutton->hasFocus()) {
                        m_listwidget->setFocus(Qt::TabFocusReason);
                    }
                    return true;
                }
                if (key_event->key() == Qt::Key_Escape) {
                    emit hideWidget();
                    return true;
                }
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

/**
 * @brief 设置焦点到该widget时转移至listwidget
 */
void MemoryWidget::focusInEvent(QFocusEvent *event)
{
    if (!m_memorypublic->isEmpty() && event->reason() == Qt::TabFocusReason) {
        m_listwidget->setCurrentRow(m_currentrow);
        m_listwidget->setFocus();
    }
    QWidget::focusInEvent(event);
}

void MemoryWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
        emit hideWidget();
    else
        return;
}

void MemoryWidget::memoryAnsChanged(int row, const Quantity answer)
{
    QString formatResult = QString();
    if (m_calculatormode == 2) {
        const QString result = programmerResult(answer);
        formatResult = Utils::formatThousandsSeparatorsPro(result, Settings::instance()->programmerBase);
    } else {
        const QString result = DMath::format(answer, Quantity::Format::General() + Quantity::Format::Precision(m_precision));
        formatResult = Utils::formatThousandsSeparators(result);
    }
    formatResult = setitemwordwrap(formatResult, row);
    MemoryItemWidget *w1 = static_cast<MemoryItemWidget *>(m_listwidget->itemWidget(m_listwidget->item(row)));
    w1->setTextLabel(formatResult);
}

/**
 * @brief MemoryWidget::memoryclean
 * 清空内存列表，同步清空quantity list
 */
void MemoryWidget::memoryclean()
{
    m_listwidget->setFocusPolicy(Qt::NoFocus); //内存无内容时NoFocus
    m_label->show();
    m_listwidget->clear();
    QListWidgetItem *item1 = new QListWidgetItem(m_listwidget);

    m_listwidget->insertItem(0, item1);
    if (m_calculatormode != 1)
        m_listwidget->item(0)->setSizeHint(QSize(m_itemwidth, m_listwidget->frameRect().height())); //标准模式
    else
        m_listwidget->item(0)->setSizeHint(QSize(m_itemwidth, SCIENTIFIC_MWIDGET_HEIGHT)); //科学模式
    m_listwidget->item(0)->setFlags(Qt::NoItemFlags);
    emptymemoryfontcolor();
    m_clearbutton->hide();
}

/**
 * @brief 内存中没有数据的字体颜色
 */
void MemoryWidget::emptymemoryfontcolor()
{
    DPalette pl = this->palette();
    if (m_themetype == 1) {
        QColor color(85, 85, 85);
        color.setAlphaF(0.4);
        pl.setColor(DPalette::WindowText, QColor(color));
    } else {
        QColor color(192, 198, 212);
        color.setAlphaF(0.4);
        pl.setColor(DPalette::WindowText, QColor(color));
    }
    m_label->setPalette(pl);
}

/**
 * 程序员专用换行
 */
QString MemoryWidget::programmerWrap(QString result)
{
    QFont font = qApp->font();
    font.setPixelSize(29);
    QFontMetrics fm(font);

    int validwidth = PRO_SCI_ITEM_WIDTH - 30;
    int l = 0, r = 0;
    int last = result.length();
    int block;
    if (16 == Settings::instance()->programmerBase || 2 == Settings::instance()->programmerBase)
        block = 5;
    else
        block = 4;

    while (r < result.length()) {
        if (fm.horizontalAdvance(result.mid(l, r - l)) < validwidth)
            ++r;
        else {
            while (result.at(r) != QLatin1String(" ") && result.at(r) != QLatin1String(",")) {
                --r;
                if (r <= l)
                    break;
            }
            if (last < r - l) {
                r -= block;
            }
            result.insert(r + 1, "\n");
            ++m_line;
            last = r - l;
            r += 2;
            l = r;
        }
    }
    return result;
}

int MemoryWidget::fontHeight()
{
    QFont font = qApp->font();
    font.setPixelSize(29);
    QFontMetrics fm(font);
    return fm.height() + 1;
}

/**
 * 字体变化需要重设高度
 */
void MemoryWidget::resetItemHeight()
{
    //程序员模式不在这里处理
    if (2 == m_calculatormode || m_memorypublic->isEmpty())
        return;
    for (int i = 0; i < m_listwidget->count(); i++) {
        if (m_listwidget->item(i)) {
            int line = static_cast<MemoryItemWidget *>(m_listwidget->itemWidget(m_listwidget->item(i)))->getLine();
            //设置item高度
            m_listwidget->item(i)->setSizeHint(QSize(m_itemwidth, 40 + fontHeight() * line));
            m_listwidget->itemWidget(m_listwidget->item(i))->setFixedSize(QSize(m_itemwidth, 40 + fontHeight() * line));
            static_cast<MemoryItemWidget *>(m_listwidget->itemWidget(m_listwidget->item(i)))->setLineHight(line, fontHeight());
        }
    }
}

/**
 * @brief MR
 */
QPair<QString, Quantity> MemoryWidget::getfirstnumber()
{
    QPair<QString, Quantity> p1;
    if (!m_memorypublic->isEmpty()) {
        MemoryItemWidget *w1 = static_cast<MemoryItemWidget *>(m_listwidget->itemWidget(m_listwidget->item(0)));
        QString str = w1->textLabel();
        p1.first = str.remove("\n");
        p1.second = m_memorypublic->getList().at(0);
        return p1;
    } else {
        p1.first = QString();
        p1.second = Quantity(0);
        return p1;
    }
}

MemoryListWidget *MemoryWidget::getMemoryWidget()
{
    return m_listwidget;
}

/**
 * @brief MemoryWidget::programmerResult
 * 根据当前进制显示相应的内存结果
 */
QString MemoryWidget::programmerResult(const Quantity answer)
{
    QString result;
    Quantity ans = DMath::integer(answer);
    switch (Settings::instance()->programmerBase) {
    case 16:
        return result = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Hexadecimal()).remove("0x");
    case 8:
        return result = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Precision(65) + Quantity::Format::Octal()).remove("0o");
    case 2:
        return result = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Precision(65) + Quantity::Format::Binary()).remove("0b");
    default:
        return result = DMath::format(ans, Quantity::Format::Complement() + Quantity::Format::Precision(65));
    }
}

/**
 * @brief MemoryWidget::resetAllLabelByBase
 * 打开内存列表时，根据当前进制刷新显示
 */
void MemoryWidget::resetAllLabelByBase()
{
    if (m_memorypublic->isEmpty())
        return;
    QString text;
    if (m_calculatormode == 2) {
        for (int i = 0; i < m_listwidget->count(); i++) {
            text = Utils::reformatSeparatorsPro(programmerResult(m_memorypublic->getList().at(i)), Settings::instance()->programmerBase);
            text = setitemwordwrap(text, i);
            static_cast<MemoryItemWidget *>(m_listwidget->itemWidget(m_listwidget->item(i)))->setTextLabel(text);
        }
    }
}



/**
 * @brief MemoryWidget::resetLabelBySeparator
 * 按分割符位数重新设置文本
 */
void MemoryWidget::resetLabelBySeparator()
{
    if (m_memorypublic->isEmpty())
        return;
    QString text;
    for (int i = 0; i < m_listwidget->count(); i++) {
        auto wgt = static_cast<MemoryItemWidget *>(m_listwidget->itemWidget(m_listwidget->item(i)));
        text = Utils::formatThousandsSeparators(wgt->textLabel().remove(","));
        wgt->setTextLabel(text);
    }
}

void MemoryWidget::initConnect()
{
    connect(m_memorypublic, &MemoryPublic::generateDataSig, this, &MemoryWidget::generateData);
    connect(m_memorypublic, &MemoryPublic::memorycleanSig, this, &MemoryWidget::memoryclean);
    connect(m_memorypublic, &MemoryPublic::memoryAnsSig, this, &MemoryWidget::memoryAnsChanged);
    connect(m_memorypublic, &MemoryPublic::widgetcleanSig, this, &MemoryWidget::widgetcleanslot);
    connect(m_memorypublic, &MemoryPublic::setThemeTypeSig, this, &MemoryWidget::setThemeType);

    connect(m_clearbutton, &TextButton::space, this, [ = ]() {
        m_memorypublic->memoryclean();
        setFocus();
    }); //focus下空格按下
    connect(m_clearbutton, &DPushButton::clicked, this, [ = ]() {
        m_memorypublic->memoryclean();
        setFocus();
    });

    connect(m_listwidget, &MemoryListWidget::itemselected, this, [ = ](int row, bool isselect) {
        if (!m_memorypublic->isEmpty() && isselect) {
            QPair<QString, Quantity> p;
            MemoryItemWidget *w1 = static_cast<MemoryItemWidget *>(m_listwidget->itemWidget(m_listwidget->item(row)));
            p.first = w1->textLabel();
            p.second = m_memorypublic->getList().at(row);
            if (m_listwidget->item(row)->flags() != Qt::NoItemFlags)
                emit itemclick(p);
        }
    });
    connect(m_listwidget, &MemoryListWidget::space, this, [ = ] {
        if (!m_memorypublic->isEmpty()) //只有listwidget在focus状态才会触发keypress,所以此处未进行hasfocus判断
        {
            QPair<QString, Quantity> p;
            int row = m_listwidget->currentRow();
            MemoryItemWidget *w1 = static_cast<MemoryItemWidget *>(m_listwidget->itemWidget(m_listwidget->item(row)));
            p.first = w1->textLabel();
            p.second = m_memorypublic->getList().at(row);
            if (m_listwidget->item(row)->flags() != Qt::NoItemFlags)
                emit itemclick(p);
        }
    });
    connect(m_listwidget, &MemoryListWidget::focus, this, [ = ](int direction) {
        switch (direction) { //只有listwidget在focus状态才会触发keypress,所以此处未进行hasfocus判断
        case 0:
            if (m_listwidget->currentRow() > 0) {
                m_listwidget->setCurrentRow(m_listwidget->currentRow() - 1);
                m_currentrow = m_listwidget->currentRow();
                m_listwidget->scrollToItem(m_listwidget->item(m_listwidget->currentRow())); //滚动条跟随焦点项
            }
            break;
        case 1:
            if (m_listwidget->currentRow() < (m_listwidget->count() - 1)) {
                m_listwidget->setCurrentRow(m_listwidget->currentRow() + 1);
                m_currentrow = m_listwidget->currentRow();
                m_listwidget->scrollToItem(m_listwidget->item(m_listwidget->currentRow())); //滚动条跟随焦点项
            }
            break;
        default:
            break;
        }
    });
    connect(m_listwidget, &MemoryListWidget::altAndM, this, [ = ]() {
        int row = m_listwidget->currentRow();
        MemoryItemWidget *w1 = static_cast<MemoryItemWidget *>(m_listwidget->itemWidget(m_listwidget->item(row)));
        w1->showTextEditMenuByAltM();
    });

    connect(qApp, &QApplication::fontChanged, this, &MemoryWidget::resetItemHeight);
}

/**
 * @brief MemoryWidget::expressionempty
 * 当输入栏为空时，内存列表中的加减按钮置灰
 * @param b
 * true = 空，false = 输入栏有内容
 */
void MemoryWidget::expressionempty(bool b)
{
    if (!m_memorypublic->isEmpty()) {
        for (int i = 0; i < m_listwidget->count(); i++) {
            static_cast<MemoryItemWidget *>(m_listwidget->itemWidget(m_listwidget->item(i)))->isexpressionempty(b);
        }
    }
}

/**
 * @brief MemoryWidget::widgetcleanslot
 * 内存列表item里的清除按钮，指定行数清除
 * @param row
 * 指定的行数
 * @param mode
 * 用于区分是标准还是科学型的删除，防止同步删除时重复删除
 */
void MemoryWidget::widgetcleanslot(int row, int mode, bool ismenu)
{
    if (m_calculatormode != mode) {
        delete m_listwidget->takeItem(row);
    } else {
        if (row + 1 < m_listwidget->count() && !ismenu)
            static_cast<MemoryItemWidget *>(m_listwidget->itemWidget(m_listwidget->item(row + 1)))->setNextItemHover();
    }
}

void MemoryWidget::setMemoryPublic(MemoryPublic *pub)
{
    m_memorypublic = pub;
    initConnect();
}

/**
 * @brief 替换text中的中文字符为英文字符
 */
QString MemoryWidget::formatExpression(const QString &text)
{
    return QString(text)
           .replace(CN_ADD, EN_ADD)
           .replace(CN_MIN, EN_MIN)
           .replace(CN_MUL, EN_MUL)
           .replace(CN_DIV, EN_DIV)
           .replace(EN_Comma, "");
}

/**
 * @brief MemoryWidget::setitemwordwrap
 * @param text
 * @param row
 * 分行函数
 * @return
 */
QString MemoryWidget::setitemwordwrap(const QString &text, int row)
{
    QString result = text;
    result.replace('-', "－").replace('+', "＋");
    int index = result.indexOf("E");
    m_line = 1;
    if (m_calculatormode == 0) {
        if (index > 0 && result.left(index).length() > 13) {
            result.insert(index, "\n");
            m_line = 2;
        } else if (index <= 0 && result.length() > 21) {
            result.insert(20, "\n");
            m_line = 2;
        }
        if (m_clearbutton->isHidden() == true) {
            m_clearbutton->show();
            m_clearbutton->updateWhenBtnDisable();
            m_clearbutton->showtooltip(true);
        }
    } else if (m_calculatormode == 1) {
        if (index > 15) {
            result.insert(index, "\n");
            m_line = 2;
            if (index > 20) {
                result.remove("\n");
                result.insert(20, "\n");
                m_line = 2;
            }
        } else if (index <= 0 && result.length() > 21) {
            result.insert(20, "\n");
            m_line = 2;
        }
    } else {
        result = programmerWrap(result);

        if (m_clearbutton->isHidden() == true) {
            m_clearbutton->show();
            m_clearbutton->updateWhenBtnDisable();
            m_clearbutton->showtooltip(true);
        }
    }
    if (m_listwidget->item(row)) {
        //设置item高度
        m_listwidget->item(row)->setSizeHint(QSize(m_itemwidth, 40 + fontHeight() * m_line));
        m_listwidget->itemWidget(m_listwidget->item(row))->setFixedSize(QSize(m_itemwidth, 40 + fontHeight() * m_line));
    }
    static_cast<MemoryItemWidget *>(m_listwidget->itemWidget(m_listwidget->item(row)))->setLineHight(m_line, fontHeight()); //设置item中label高度
    return result;
}

/**
 * @brief 根据主题变换垃圾桶及内存中没有数据字体颜色
 */
void MemoryWidget::setThemeType(int type)
{
    m_listwidget->update(); //刷新widget
    int typeIn = type;
    if (typeIn == 0) {
        typeIn = DGuiApplicationHelper::instance()->themeType();
    }
    m_themetype = typeIn;
    emit themechange(m_themetype);
    QString path;
    if (m_themetype == 1) {
        path = QString(":/assets/images/%1/").arg("light");
        m_clearbutton->setIconUrl(path + "empty_normal.svg", path + "empty_hover.svg", path + "empty_press.svg", 1);
        if (m_memorypublic->isEmpty()) {
            emptymemoryfontcolor(); //更改内存中没有数据字体颜色
        }
    } else {
        path = QString(":/assets/images/%1/").arg("dark");
        m_clearbutton->setIconUrl(path + "empty_normal.svg", path + "empty_hover.svg", path + "empty_press.svg", 1);
        if (m_memorypublic->isEmpty()) {
            emptymemoryfontcolor(); //更改内存中没有数据字体颜色
        }
    }
}

MemoryWidget::~MemoryWidget()
{
}
