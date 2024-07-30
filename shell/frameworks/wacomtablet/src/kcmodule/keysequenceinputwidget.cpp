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

#include "keysequenceinputwidget.h"

#include "keysequenceinputbutton.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QPushButton>
#include <QToolButton>

using namespace Wacom;

namespace Wacom
{
class KeySequenceInputWidgetPrivate
{
public:
    KeySequenceInputWidgetPrivate(KeySequenceInputWidget *q);
    ~KeySequenceInputWidgetPrivate();

    void setupUi();

    KeySequenceInputWidget *const q;

    QHBoxLayout *layout;
    KeySequenceInputButton *keyButton;
    QToolButton *clearButton;
};
}

KeySequenceInputWidgetPrivate::KeySequenceInputWidgetPrivate(KeySequenceInputWidget *q)
    : q(q)
    , layout(nullptr)
    , keyButton(nullptr)
    , clearButton(nullptr)
{
    setupUi();
}

KeySequenceInputWidgetPrivate::~KeySequenceInputWidgetPrivate()
{
    // must exist, even if not used.
}

void KeySequenceInputWidgetPrivate::setupUi()
{
    layout = new QHBoxLayout(q);
    layout->setContentsMargins({});

    keyButton = new KeySequenceInputButton(q);
    layout->addWidget(keyButton);

    clearButton = new QToolButton(q);
    layout->addWidget(clearButton);

    if (qApp->isLeftToRight()) {
        clearButton->setIcon(QIcon::fromTheme(QLatin1String("edit-clear-locationbar-rtl")));
    } else {
        clearButton->setIcon(QIcon::fromTheme(QLatin1String("edit-clear-locationbar-ltr")));
    }
}

KeySequenceInputWidget::KeySequenceInputWidget(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new KeySequenceInputWidgetPrivate(this))
{
    Q_D(KeySequenceInputWidget);

    this->setLayout(d->layout);
    connect(d->clearButton, SIGNAL(clicked()), this, SLOT(clearKeySequence()));
    connect(d->keyButton, SIGNAL(keySequenceChanged(QKeySequence)), this, SLOT(onKeySequenceChanged(QKeySequence)));
}

KeySequenceInputWidget::~KeySequenceInputWidget()
{
    // destructor must exist in cpp file, even if not used!
}

const QKeySequence &KeySequenceInputWidget::keySequence() const
{
    Q_D(const KeySequenceInputWidget);

    return d->keyButton->getSequence();
}

void KeySequenceInputWidget::clearKeySequence()
{
    Q_D(KeySequenceInputWidget);
    d->keyButton->clearSequence();
    onKeySequenceChanged(d->keyButton->getSequence());
}

void KeySequenceInputWidget::setKeySequence(const QKeySequence &sequence)
{
    Q_D(KeySequenceInputWidget);
    d->keyButton->setSequence(sequence);
    onKeySequenceChanged(d->keyButton->getSequence());
}

void KeySequenceInputWidget::onKeySequenceChanged(const QKeySequence &sequence)
{
    emit keySequenceChanged(sequence);
}

#include "moc_keysequenceinputwidget.cpp"
