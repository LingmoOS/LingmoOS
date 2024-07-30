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

#include "buttonactionselectorwidget.h"
#include "ui_buttonactionselectorwidget.h"

#include "buttonactionselectiondialog.h"

#include <QIcon>

using namespace Wacom;

namespace Wacom
{
class ButtonActionSelectorWidgetPrivate
{
public:
    ButtonActionSelectorWidgetPrivate()
        : ui(new Ui::ButtonActionSelectorWidget)
    {
    }
    ~ButtonActionSelectorWidgetPrivate()
    {
        delete ui;
    }

    ButtonShortcut shortcut;
    Ui::ButtonActionSelectorWidget *ui;
};
}

ButtonActionSelectorWidget::ButtonActionSelectorWidget(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new ButtonActionSelectorWidgetPrivate)
{
    setupUi();
}

ButtonActionSelectorWidget::~ButtonActionSelectorWidget()
{
    delete this->d_ptr;
}

const ButtonShortcut &ButtonActionSelectorWidget::getShortcut() const
{
    Q_D(const ButtonActionSelectorWidget);
    return d->shortcut;
}

void ButtonActionSelectorWidget::setShortcut(const ButtonShortcut &shortcut)
{
    Q_D(ButtonActionSelectorWidget);
    d->shortcut = shortcut;

    updateSelectorButton(shortcut);
    updateActionName(shortcut);
}

void ButtonActionSelectorWidget::onButtonActionSelectorClicked()
{
    Q_D(ButtonActionSelectorWidget);

    ButtonActionSelectionDialog selectorDialog;

    selectorDialog.setShortcut(d->shortcut);
    selectorDialog.exec();

    ButtonShortcut selectedShortcut(selectorDialog.getShortcut());

    if (d->shortcut != selectedShortcut) {
        setShortcut(selectedShortcut);
        emit buttonActionChanged(d->shortcut);
    }
}

void ButtonActionSelectorWidget::onLineEditSelectionChanged()
{
    Q_D(ButtonActionSelectorWidget);
    d->ui->actionNameDisplayWidget->deselect();
}

void ButtonActionSelectorWidget::setupUi()
{
    Q_D(ButtonActionSelectorWidget);

    d->ui->setupUi(this);

    connect(d->ui->actionSelectionButton, SIGNAL(clicked(bool)), this, SLOT(onButtonActionSelectorClicked()));
    connect(d->ui->actionNameDisplayWidget, SIGNAL(selectionChanged()), this, SLOT(onLineEditSelectionChanged()));
    connect(d->ui->actionNameDisplayWidget, SIGNAL(mousePressed()), this, SLOT(onButtonActionSelectorClicked()));

    setShortcut(ButtonShortcut());
}

void ButtonActionSelectorWidget::updateActionName(const ButtonShortcut &shortcut)
{
    Q_D(ButtonActionSelectorWidget);

    QString displayName = shortcut.toDisplayString();

    d->ui->actionNameDisplayWidget->blockSignals(true);
    d->ui->actionNameDisplayWidget->setText(displayName);
    d->ui->actionNameDisplayWidget->blockSignals(false);
}

void ButtonActionSelectorWidget::updateSelectorButton(const ButtonShortcut &shortcut)
{
    Q_D(ButtonActionSelectorWidget);

    if (shortcut.isButton()) {
        // shortcut is a mouse button click
        d->ui->actionSelectionButton->setIcon(QIcon::fromTheme(QLatin1String("input-mouse")));

    } else if (shortcut.isModifier() || shortcut.isKeystroke()) {
        // shortcut is a keyboard action
        d->ui->actionSelectionButton->setIcon(QIcon::fromTheme(QLatin1String("input-keyboard")));

    } else {
        // shortcut not set or unknown
        d->ui->actionSelectionButton->setIcon(QIcon::fromTheme(QLatin1String("configure")));
    }
}

#include "moc_buttonactionselectorwidget.cpp"
