/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include <QPaintEvent>
#include <QPainter>
#include <QPolygon>
#include <QLineEdit>
#include <QAbstractItemView>

#include "icon-view-editor.h"

using namespace Peony;
using namespace DirectoryView;

IconViewEditor::IconViewEditor(QWidget *parent) : QTextEdit(parent)
{
    setAcceptRichText(false);
    //setContextMenuPolicy(Qt::CustomContextMenu);
    m_styled_edit = new QLineEdit;
    setContentsMargins(0, 0, 0, 0);
    setAlignment(Qt::AlignCenter);
    // fix #164278, icon view text editor doesn't cover view item.
    // note on lingmo platform theme, style panel frame is not visible,
    // we have to draw a frame by ourselves.
    setFrameShape(QFrame::NoFrame);

//    setStyleSheet("padding: 0px;"
//                  "background-color: white;");

    connect(this, &IconViewEditor::textChanged, this, &IconViewEditor::minimalAdjust);
}

IconViewEditor::~IconViewEditor()
{
    m_styled_edit->deleteLater();
}

void IconViewEditor::paintEvent(QPaintEvent *e)
{
    QPainter p(this->viewport());
    qreal padding = 1.0;
    QRectF rectF = viewport()->rect();
    p.save();
    if (devicePixelRatioF() < 2.0) {
        if (devicePixelRatioF() > 1.0) {
            padding = qMin(devicePixelRatioF(), 1.5);
            p.setRenderHint(QPainter::Antialiasing);
        }
    }
    p.fillRect(rectF, palette().highlight());
    p.fillRect(rectF.adjusted(padding, padding, -padding, -padding), palette().base());
    p.restore();
    QTextEdit::paintEvent(e);
}

void IconViewEditor::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
        Q_EMIT returnPressed();
        return;
    }
    QTextEdit::keyPressEvent(e);
}

void IconViewEditor::minimalAdjust()
{
    if (m_max_length_limit) {
        //fix #154584
        blockSignals(true);
        auto position = textCursor().position();
        while (true) {
            if (m_limit_bytes) {
                auto local8Bit = toPlainText().toLocal8Bit();
                if (local8Bit.length() <= m_max_length_limit) {
                    break;
                }
            } else {
                if (toPlainText().length() <= m_max_length_limit) {
                    break;
                }
            }
            if (position > 0) {
                position--;
                textCursor().beginEditBlock();
                textCursor().setPosition(position);
                textCursor().deletePreviousChar();
                textCursor().endEditBlock();
            } else {
                break;
            }
        }
        blockSignals(false);
    }

    this->resize(QSize(document()->size().width(), document()->size().height() + 24));
}

void IconViewEditor::setMaxLengthLimit(int length)
{
    m_max_length_limit = length;
}

void IconViewEditor::setLimitBytes(bool limitBytes)
{
    m_limit_bytes = limitBytes;
}
