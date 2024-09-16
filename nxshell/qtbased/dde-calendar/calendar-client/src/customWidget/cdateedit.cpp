// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "cdateedit.h"
#include "lunarcalendarwidget.h"
#include "lunarmanager.h"

#include <QCoreApplication>
#include <QLineEdit>
#include <QDebug>

CDateEdit::CDateEdit(QWidget *parent) : QDateEdit(parent)
{
    connect(this, &QDateEdit::userDateChanged, this, &CDateEdit::slotDateEidtInfo);
    connect(lineEdit(), &QLineEdit::textChanged, this, &CDateEdit::slotRefreshLineEditTextFormat);
    connect(lineEdit(), &QLineEdit::cursorPositionChanged, this, &CDateEdit::slotCursorPositionChanged);
    connect(lineEdit(), &QLineEdit::selectionChanged, this, &CDateEdit::slotSelectionChanged);
    connect(lineEdit(), &QLineEdit::editingFinished, this, [this]() {
        //监听完成输入信号，触发文本改变事件，保证退出文本编辑的情况下依旧能刷新文本样式
        //当非手动输入时间时不会触发文本改变信号
        slotRefreshLineEditTextFormat(text());
    });
}

void CDateEdit::setDate(QDate date)
{
    QDateEdit::setDate(date);
    //只有在农历日程时，才需要获取农历信息
    QString dtFormat = m_showLunarCalendar ? m_format + getLunarName(date) : m_format;
    m_strCurrrentDate = date.toString(dtFormat);
}

void CDateEdit::setDisplayFormat(QString format)
{
    this->m_format = format;
    //刷新时间显示信息
    slotDateEidtInfo(date());
}

QString CDateEdit::displayFormat()
{
    return m_format;
}

void CDateEdit::setLunarCalendarStatus(bool status)
{
    m_showLunarCalendar = status;
    //刷新时间显示信息
    slotDateEidtInfo(date());
    //更新日历显示类型
    updateCalendarWidget();
}

void CDateEdit::setLunarTextFormat(QTextCharFormat format)
{
    m_lunarTextFormat = format;
    //刷新文本样式
    slotRefreshLineEditTextFormat(text());
}

QTextCharFormat CDateEdit::getsetLunarTextFormat()
{
    return m_lunarTextFormat;
}

void CDateEdit::setCalendarPopup(bool enable)
{
    QDateEdit::setCalendarPopup(enable);
    //更新日历显示类型
    updateCalendarWidget();
}

void CDateEdit::slotDateEidtInfo(const QDate &date)
{
    QString format = m_format;


    if (m_showLunarCalendar) {
        if (!showGongli()) {
            format = "yyyy/";
        }
        m_lunarName = getLunarName(date);
        format += m_lunarName;
    }
    //当当前显示格式与应该显示格式一致时不再重新设置
    if (QDateEdit::displayFormat() == format) {
        return;
    }

    //记录刷新格式前的状态
    bool hasSelected = lineEdit()->hasSelectedText();   //是否选择状态
    int cPos = 0;
    QDateTimeEdit::Section section = QDateTimeEdit::NoSection;
    if (hasSelected) {
        section = currentSection();     //选择节
    } else {
        cPos = lineEdit()->cursorPosition();    //光标所在位置
    }

    QDateEdit::setDisplayFormat(format);

    //恢复原状
    if (hasSelected) {
        setSelectedSection(section);    //设置选中节
    } else {
        lineEdit()->setCursorPosition(cPos);    //设置光标位置
    }
    //刷新文本样式
    //当非手动输入时间时不会触发文本改变信号
    slotRefreshLineEditTextFormat(date.toString(format));
}

void CDateEdit::slotRefreshLineEditTextFormat(const QString &text)
{
    QFont font = lineEdit()->font();
    QFontMetrics fm(font);
    int textWidth = fm.width(text);
    int maxWidth = lineEdit()->width() - 25; //文本能正常显示的最大宽度
    if (textWidth > maxWidth) {
        setToolTip(text);
    } else {
        setToolTip("");
    }

    //不显示农历时无需处理
    if (!m_showLunarCalendar) {
        return;
    }

    QList<QTextLayout::FormatRange> formats;

    QTextLayout::FormatRange fr_tracker;

    fr_tracker.start = text.size() - m_lunarName.size();    //样式起始位置
    fr_tracker.length = m_lunarName.size();                 //样式长度

    fr_tracker.format = m_lunarTextFormat;                  //样式

    formats.append(fr_tracker);
    //更改农历文本样式
    setLineEditTextFormat(lineEdit(), formats);
}

void CDateEdit::slotCursorPositionChanged(int oldPos, int newPos)
{
    //不显示农历时无需处理
    if (!m_showLunarCalendar) {
        return;
    }
    Q_UNUSED(oldPos);

    //光标最大位置不能超过时间长度不能覆盖农历信息
    int maxPos = text().length() - m_lunarName.length();
    bool hasSelected = lineEdit()->hasSelectedText();

    if (hasSelected) {
        int startPos = lineEdit()->selectionStart();
        int endPos = lineEdit()->selectionEnd();

        //新的光标位置与选择区域末尾位置相等则是向后选择，向前选择无需处理
        if (newPos == endPos) {
            newPos = newPos > maxPos ? maxPos : newPos;
            lineEdit()->setSelection(startPos, newPos - startPos); //重新设置选择区域
        }
    } else {
        //非选择情况当新光标位置大于最大位置时设置到最大位置处，重新设置选中节位最后一节
        if (newPos > maxPos) {
            setCurrentSectionIndex(sectionCount() - 1);
            lineEdit()->setCursorPosition(maxPos);
        }
    }
}

void CDateEdit::slotSelectionChanged()
{
    //不显示农历时无需处理
    if (!m_showLunarCalendar) {
        return;
    }
    //全选时重新设置为只选择时间不选择农历
    if (lineEdit()->hasSelectedText() && lineEdit()->selectionEnd() == text().length()) {
        int startPos = lineEdit()->selectionStart();
        lineEdit()->setSelection(startPos, text().length() - m_lunarName.length() - startPos);
    }
}

QString CDateEdit::getLunarName(const QDate &date)
{
    return gLunarManager->getHuangLiShortName(date);
}

void CDateEdit::setLineEditTextFormat(QLineEdit *lineEdit, const QList<QTextLayout::FormatRange> &formats)
{
    if (!lineEdit) {
        return;
    }
    QList<QInputMethodEvent::Attribute> attributes;

    for (const QTextLayout::FormatRange &fr : formats) {
        QInputMethodEvent::AttributeType type = QInputMethodEvent::TextFormat;

        int start = fr.start - lineEdit->cursorPosition();
        int length = fr.length;
        QVariant value = fr.format;

        attributes.append(QInputMethodEvent::Attribute(type, start, length, value));
    }

    QInputMethodEvent event(QString(), attributes);

    QCoreApplication::sendEvent(lineEdit, &event);
}

void CDateEdit::changeEvent(QEvent *e)
{
    QDateEdit::changeEvent(e);
    if (e->type() == QEvent::FontChange && m_showLunarCalendar) {
        slotDateEidtInfo(date());
    }
}

bool CDateEdit::showGongli()
{
    QString str = m_strCurrrentDate;
    QFontMetrics fontMetrice(lineEdit()->font());
    if (fontMetrice.width(str) > lineEdit()->width() - 20) {
        return false;
    }
    return true;
}

void CDateEdit::updateCalendarWidget()
{
    if (calendarPopup()) {
        //setCalendarWidget:
        //The editor does not automatically take ownership of the calendar widget.
        if (m_showLunarCalendar) {
            setCalendarWidget(new LunarCalendarWidget(this));
        } else {
            setCalendarWidget(new QCalendarWidget(this));
        }
    }
}

void CDateEdit::setEditCursorPos(int pos)
{
    QLineEdit *edit = lineEdit();
    if (nullptr != edit) {
        edit->setCursorPosition(pos);
    }
}

