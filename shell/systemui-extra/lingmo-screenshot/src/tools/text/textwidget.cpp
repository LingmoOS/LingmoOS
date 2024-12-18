/* Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
*               2020 KylinSoft Co., Ltd.
* This file is part of Lingmo-Screenshot.
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "textwidget.h"

TextWidget::TextWidget(QWidget *parent) : QTextEdit(parent)
{
    setStyleSheet(QStringLiteral("TextWidget { background: transparent; }"));
    connect(this, &TextWidget::textChanged,
            this, &TextWidget::adjustSize);
    connect(this, &TextWidget::textChanged,
            this, &TextWidget::emitTextUpdated);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setContextMenuPolicy(Qt::NoContextMenu);
}

void TextWidget::setMaxSize(int max_Width, int max_Height)
{
    max_width = max_Width;
    max_height = max_Height;
}

void TextWidget::showEvent(QShowEvent *e)
{
    QFont font;
    QFontMetrics fm(font);
    setFixedWidth(fm.lineSpacing() *6);
    setFixedHeight(fm.lineSpacing() * 2.5);
    m_baseSize = size();
    m_minSize = m_baseSize;
    QTextEdit::showEvent(e);
    adjustSize();
}

void TextWidget::resizeEvent(QResizeEvent *e)
{
    m_minSize.setHeight(qMin(m_baseSize.height(), height()));
    m_minSize.setWidth(qMin(m_baseSize.width(), width()));
    QTextEdit::resizeEvent(e);
}

void TextWidget::setFont(const QFont &f)
{
    QTextEdit::setFont(f);
    adjustSize();
}

void TextWidget::updateFont(const QFont &f)
{
    setFont(f);
}

void TextWidget::setFontPointSize(qreal s)
{
    QFont f = font();
    f.setPointSize(s);
    setFont(f);
}

void TextWidget::setTextColor(const QColor &c)
{
    QString s(QStringLiteral("TextWidget { background: transparent; color: %1; }"));
    setStyleSheet(s.arg(c.name()));
}

void TextWidget::adjustSize()
{
    QString &&text = this->toPlainText();

    QFontMetrics fm(font());
    QRect bounds = fm.boundingRect(QRect(), 0, text);
    int pixelsWide = bounds.width() + fm.lineSpacing();
    int pixelsHigh = bounds.height() * 1.15 + fm.lineSpacing();
    pixelsWide = qBound(5, pixelsWide, max_width);
    pixelsHigh = max_height;
    this->setFixedSize(pixelsWide, pixelsHigh);
}

void TextWidget::emitTextUpdated()
{
    emit textUpdated(this->toPlainText());
}
