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

#include "hwbuttondialog.h"
#include "ui_hwbuttondialog.h"

#include <QMouseEvent>
#include <QPushButton>

static unsigned int QtButton2X11Button(Qt::MouseButton qtbutton)
{
    // We could probably just use log2 here, but I don't know if this can backfire
    // Qt seems to offer no function for getting index of a set flag
    unsigned int button = qtbutton;
    unsigned int buttonNumber = 0;
    while (button > 0) {
        buttonNumber++;
        button >>= 1;
    }

    if (buttonNumber < 4) {
        return buttonNumber;
    } else { // X11 buttons 4-7 are reserved for scroll wheel
        return buttonNumber + 4;
    }
}

using namespace Wacom;

HWButtonDialog::HWButtonDialog(int maxButtons, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::HWButtonDialog)
    , m_maxButtons(maxButtons)
    , m_nextButton(1)
{
    ui->setupUi(this);

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    nextButton();
}

HWButtonDialog::~HWButtonDialog()
{
    delete ui;
}

void HWButtonDialog::nextButton()
{
    if (m_nextButton <= m_maxButtons) {
        QString text = i18n("Please press Button %1 on the tablet now.", m_nextButton);
        text.prepend(QLatin1String("<b>"));
        text.append(QLatin1String("</b>"));
        ui->textEdit->insertHtml(text);
        ui->textEdit->insertHtml(QLatin1String("<br>"));

        QTextCursor cursor = ui->textEdit->textCursor();
        cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
        ui->textEdit->setTextCursor(cursor);
    }
}

void HWButtonDialog::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MouseButton::NoButton) {
        return;
    }

    hwKey(QtButton2X11Button(event->button()));
}

void HWButtonDialog::hwKey(unsigned int button)
{
    if (m_nextButton <= m_maxButtons) {
        QString text = i18n("Hardware button %1 detected.", button);
        ui->textEdit->insertHtml(text);
        ui->textEdit->insertHtml(QLatin1String("<br><br>"));
        QTextCursor cursor = ui->textEdit->textCursor();
        cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
        ui->textEdit->setTextCursor(cursor);

        m_buttonMap << button;
        m_nextButton++;
    }

    if (m_nextButton > m_maxButtons) {
        ui->textEdit->insertHtml(i18n("All buttons detected. Please close dialog"));
        ui->textEdit->insertHtml(QLatin1String("<br>"));

        QTextCursor cursor = ui->textEdit->textCursor();
        cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
        ui->textEdit->setTextCursor(cursor);

        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    } else {
        nextButton();
    }
}

QList<unsigned int> HWButtonDialog::buttonMap() const
{
    return m_buttonMap;
}

#include "moc_hwbuttondialog.cpp"
