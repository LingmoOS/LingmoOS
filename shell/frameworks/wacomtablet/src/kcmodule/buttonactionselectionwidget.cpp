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

#include "buttonactionselectionwidget.h"
#include "ui_buttonactionselectionwidget.h"

#include "buttonshortcut.h"

#include <QString>

using namespace Wacom;

namespace Wacom
{
class ButtonActionSelectionWidgetPrivate
{
public:
    ButtonActionSelectionWidgetPrivate()
        : ui(new Ui::ButtonActionSelectionWidget)
    {
    }
    ~ButtonActionSelectionWidgetPrivate()
    {
        delete ui;
    }

    Ui::ButtonActionSelectionWidget *ui;
    ButtonShortcut shortcut;
};
}

ButtonActionSelectionWidget::ButtonActionSelectionWidget(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new ButtonActionSelectionWidgetPrivate)
{
    setupUi();
}

ButtonActionSelectionWidget::~ButtonActionSelectionWidget()
{
    delete this->d_ptr;
}

const ButtonShortcut &ButtonActionSelectionWidget::getShortcut() const
{
    Q_D(const ButtonActionSelectionWidget);
    return d->shortcut;
}

void ButtonActionSelectionWidget::setShortcut(const ButtonShortcut &shortcut)
{
    Q_D(ButtonActionSelectionWidget);

    d->shortcut = shortcut;

    updateMouseButtonSeletion(shortcut);
    updateModifierWidgets(shortcut);
    updateShortcutWidgets(shortcut);
    updateCurrentActionName(shortcut);
}

void ButtonActionSelectionWidget::onClearButtonClicked(bool checked)
{
    Q_UNUSED(checked);

    setShortcut(ButtonShortcut());
}

void ButtonActionSelectionWidget::onActionLineEditSelectionChanged()
{
    Q_D(ButtonActionSelectionWidget);
    d->ui->actionNameLineEdit->deselect();
}

void ButtonActionSelectionWidget::onModifierChanged(int state)
{
    Q_D(const ButtonActionSelectionWidget);

    Q_UNUSED(state);

    // build new shortcut sequence
    QString shortcutString;

    if (d->ui->ctrlModifierCheckBox->isChecked()) {
        shortcutString.append(QString::fromLatin1(" %1").arg(QLatin1String(" Ctrl")));
    }

    if (d->ui->altModifierCheckBox->isChecked()) {
        shortcutString.append(QString::fromLatin1(" %1").arg(QLatin1String(" Alt")));
    }

    if (d->ui->metaModifierCheckBox->isChecked()) {
        shortcutString.append(QString::fromLatin1(" %1").arg(QLatin1String(" Meta")));
    }

    if (d->ui->shiftModifierCheckBox->isChecked()) {
        shortcutString.append(QString::fromLatin1(" %1").arg(QLatin1String(" Shift")));
    }

    setShortcut(ButtonShortcut(shortcutString));
}

void ButtonActionSelectionWidget::onMouseSelectionChanged(int index)
{
    Q_D(const ButtonActionSelectionWidget);

    int button = d->ui->mouseComboBox->itemData(index).toInt();
    setShortcut(ButtonShortcut(button));
}

void ButtonActionSelectionWidget::onShortcutChanged(QKeySequence sequence)
{
    // By adding the "key" modifier, we make sure that shortcuts
    // like "1" do not get mistaken as a mouse button action.
    setShortcut(ButtonShortcut(QString::fromLatin1("key %1").arg(sequence.toString())));
}

void ButtonActionSelectionWidget::setupUi()
{
    Q_D(ButtonActionSelectionWidget);

    // setup main widget
    d->ui->setupUi(this);

    // set mouse and keyboard label icons
    d->ui->mouseIconLabel->setPixmap(QIcon::fromTheme(QLatin1String("input-mouse")).pixmap(QSize(48, 48)));
    d->ui->keyboardIconLabel->setPixmap(QIcon::fromTheme(QLatin1String("input-keyboard")).pixmap(QSize(48, 48)));

    // add mouse buttons to dropdown menu
    d->ui->mouseComboBox->addItem(i18nc("Select a mouse button from a dropwdown.", "Click to select..."), 0);

    ButtonShortcut shortcut;
    for (int i = 1; i < 33; ++i) {
        shortcut.setButton(i);
        d->ui->mouseComboBox->addItem(shortcut.toDisplayString(), i);
    }

    // add icons to clear buttons (the icon direction must be the layout direction inverted)
    QLatin1String clearIconStr("edit-clear-locationbar-rtl");
    if (!qApp->isLeftToRight())
        clearIconStr = QLatin1String("edit-clear-locationbar-ltr");
    d->ui->mouseClearButton->setIcon(QIcon::fromTheme(clearIconStr));
    d->ui->modifierClearButton->setIcon(QIcon::fromTheme(clearIconStr));

    connect(d->ui->mouseComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onMouseSelectionChanged(int)));
    connect(d->ui->mouseClearButton, SIGNAL(clicked(bool)), this, SLOT(onClearButtonClicked(bool)));

    connect(d->ui->ctrlModifierCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onModifierChanged(int)));
    connect(d->ui->altModifierCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onModifierChanged(int)));
    connect(d->ui->metaModifierCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onModifierChanged(int)));
    connect(d->ui->shiftModifierCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onModifierChanged(int)));
    connect(d->ui->modifierClearButton, SIGNAL(clicked(bool)), this, SLOT(onClearButtonClicked(bool)));

    connect(d->ui->shortcutSelectorWidget, SIGNAL(keySequenceChanged(QKeySequence)), this, SLOT(onShortcutChanged(QKeySequence)));

    connect(d->ui->actionNameLineEdit, SIGNAL(selectionChanged()), this, SLOT(onActionLineEditSelectionChanged()));

    setShortcut(ButtonShortcut());
}

void ButtonActionSelectionWidget::updateCurrentActionName(const ButtonShortcut &shortcut)
{
    Q_D(ButtonActionSelectionWidget);

    // update current action
    d->ui->actionNameLineEdit->setText(shortcut.toDisplayString());
}

void ButtonActionSelectionWidget::updateModifierWidgets(const ButtonShortcut &shortcut)
{
    Q_D(ButtonActionSelectionWidget);

    if (shortcut.isModifier()) {
        // shortcut is a modifier sequence - set checkboxes accordingly
        QString shortcutString = shortcut.toString();
        bool isChecked = false;

        isChecked = shortcutString.contains(QLatin1String("ctrl"), Qt::CaseInsensitive);
        updateQCheckBox(*(d->ui->ctrlModifierCheckBox), isChecked);

        isChecked = shortcutString.contains(QLatin1String("alt"), Qt::CaseInsensitive);
        updateQCheckBox(*(d->ui->altModifierCheckBox), isChecked);

        isChecked =
            (shortcutString.contains(QLatin1String("super"), Qt::CaseInsensitive) || shortcutString.contains(QLatin1String("meta"), Qt::CaseInsensitive));
        updateQCheckBox(*(d->ui->metaModifierCheckBox), isChecked);

        isChecked = shortcutString.contains(QLatin1String("shift"), Qt::CaseInsensitive);
        updateQCheckBox(*(d->ui->shiftModifierCheckBox), isChecked);

    } else {
        // not a modifier shortcut
        updateQCheckBox(*(d->ui->ctrlModifierCheckBox), false);
        updateQCheckBox(*(d->ui->altModifierCheckBox), false);
        updateQCheckBox(*(d->ui->metaModifierCheckBox), false);
        updateQCheckBox(*(d->ui->shiftModifierCheckBox), false);
    }
}

void ButtonActionSelectionWidget::updateMouseButtonSeletion(const ButtonShortcut &shortcut)
{
    Q_D(ButtonActionSelectionWidget);

    // find new selection index for the given button
    int newIndex = d->ui->mouseComboBox->findData(shortcut.getButton());

    // update combo box selection if button is not yet selected
    if (newIndex != d->ui->mouseComboBox->currentIndex() && d->ui->mouseComboBox->count() > 0) {
        d->ui->mouseComboBox->blockSignals(true);
        d->ui->mouseComboBox->setCurrentIndex(newIndex >= 0 ? newIndex : 0);
        d->ui->mouseComboBox->blockSignals(false);
    }
}

void ButtonActionSelectionWidget::updateQCheckBox(QCheckBox &checkbox, bool isChecked) const
{
    if (checkbox.isChecked() != isChecked) {
        checkbox.blockSignals(true);
        checkbox.setChecked(isChecked);
        checkbox.blockSignals(false);
    }
}

void ButtonActionSelectionWidget::updateShortcutWidgets(const ButtonShortcut &shortcut)
{
    Q_D(ButtonActionSelectionWidget);

    if (shortcut.isKeystroke()) {
        // shortcut is a key sequence - update it if not yet set
        QKeySequence qkeySequence = QKeySequence::fromString(shortcut.toQKeySequenceString());

        if (d->ui->shortcutSelectorWidget->keySequence() != qkeySequence) {
            d->ui->shortcutSelectorWidget->blockSignals(true);
            d->ui->shortcutSelectorWidget->setKeySequence(qkeySequence);
            d->ui->shortcutSelectorWidget->blockSignals(false);
        }

    } else {
        // not a keyboard shortcut - clear sequence if not yet cleared
        if (!d->ui->shortcutSelectorWidget->keySequence().isEmpty()) {
            d->ui->shortcutSelectorWidget->blockSignals(true);
            d->ui->shortcutSelectorWidget->clearKeySequence();
            d->ui->shortcutSelectorWidget->blockSignals(false);
        }
    }
}

#include "moc_buttonactionselectionwidget.cpp"
