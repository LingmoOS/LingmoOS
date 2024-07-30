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

#include "buttonactionselectiondialog.h"

#include "buttonactionselectionwidget.h"
#include "buttonshortcut.h"

#include <KLocalizedString>
#include <QAbstractButton>
#include <QDialogButtonBox>
#include <QIcon>
#include <QVBoxLayout>

using namespace Wacom;

namespace Wacom
{
class ButtonActionSelectionDialogPrivate
{
public:
    ButtonShortcut shortcut;
    ButtonActionSelectionWidget *selectionWidget; // no need to delete this widget as it is properly parented.
};
}

ButtonActionSelectionDialog::ButtonActionSelectionDialog(QWidget *parent)
    : QDialog(parent)
    , d_ptr(new ButtonActionSelectionDialogPrivate)
{
    Q_D(ButtonActionSelectionDialog);

    d->selectionWidget = new ButtonActionSelectionWidget(this);

    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(d->selectionWidget);
    layout->addWidget(buttonBox);

    setWindowTitle(i18nc("The action that will be assigned to a tablet button.", "Select Button Action"));
    setWindowIcon(QIcon::fromTheme(QLatin1String("preferences-desktop-tablet")));

    connect(buttonBox, &QDialogButtonBox::clicked, [this, buttonBox](QAbstractButton *button) {
        if (QDialogButtonBox::Ok == buttonBox->standardButton(button)) {
            onOkClicked();
            accept();
        } else {
            // Cancel
            reject();
        }
    });
}

ButtonActionSelectionDialog::~ButtonActionSelectionDialog()
{
    delete this->d_ptr;
}

const ButtonShortcut &ButtonActionSelectionDialog::getShortcut() const
{
    Q_D(const ButtonActionSelectionDialog);
    return d->shortcut;
}

void ButtonActionSelectionDialog::setShortcut(const ButtonShortcut &shortcut)
{
    Q_D(ButtonActionSelectionDialog);
    d->shortcut = shortcut;
    d->selectionWidget->setShortcut(shortcut);
}

void ButtonActionSelectionDialog::onOkClicked()
{
    Q_D(ButtonActionSelectionDialog);
    d->shortcut = d->selectionWidget->getShortcut();
}

#include "moc_buttonactionselectiondialog.cpp"
