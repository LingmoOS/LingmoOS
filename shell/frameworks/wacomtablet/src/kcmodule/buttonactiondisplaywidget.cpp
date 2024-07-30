/*
 * This file is part of the KDE wacomtablet project. For copyright
 * information and license terms see the AUTHORS and COPYING files
 * in the top-level directory of this distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "buttonactiondisplaywidget.h"

using namespace Wacom;

ButtonActionDisplayWidget::ButtonActionDisplayWidget(QWidget *parent)
    : QLineEdit(parent)
{
    // nothing to do
}

ButtonActionDisplayWidget::~ButtonActionDisplayWidget()
{
    // nothing to do
}

void ButtonActionDisplayWidget::focusInEvent(QFocusEvent *e)
{
    // we do not want to have focus as this would clear the placeholder text.
    QLineEdit::focusInEvent(e);
    clearFocus();
}

void ButtonActionDisplayWidget::mousePressEvent(QMouseEvent *e)
{
    QLineEdit::mousePressEvent(e);
    emit mousePressed();
}

#include "moc_buttonactiondisplaywidget.cpp"
