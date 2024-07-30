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

#include "tabletareaselectiondialog.h"

#include "tabletareaselectionwidget.h"

#include "screensinfo.h"
#include "stringutils.h"

#include <KLocalizedString>

#include <QDialogButtonBox>
#include <QIcon>
#include <QVBoxLayout>

using namespace Wacom;

namespace Wacom
{
class TabletAreaSelectionDialogPrivate
{
public:
    TabletAreaSelectionWidget *selectionWidget = nullptr; // no need to delete this widget as it is properly parented.
}; // PRIVATE CLASS
} // NAMESPACE

TabletAreaSelectionDialog::TabletAreaSelectionDialog()
    : QDialog(nullptr)
    , d_ptr(new TabletAreaSelectionDialogPrivate)
{
    setupUi();
}

TabletAreaSelectionDialog::~TabletAreaSelectionDialog()
{
    delete this->d_ptr;
}

const ScreenMap &TabletAreaSelectionDialog::getScreenMap()
{
    Q_D(TabletAreaSelectionDialog);

    return d->selectionWidget->getScreenMap();
}

const ScreenSpace TabletAreaSelectionDialog::getScreenSpace() const
{
    Q_D(const TabletAreaSelectionDialog);

    return d->selectionWidget->getScreenSpace();
}

void TabletAreaSelectionDialog::select(QString output)
{
    Q_D(TabletAreaSelectionDialog);

    d->selectionWidget->select(output);
}

void TabletAreaSelectionDialog::select(const ScreenSpace &screenSpace)
{
    Q_D(TabletAreaSelectionDialog);

    d->selectionWidget->select(screenSpace);
}

void TabletAreaSelectionDialog::setupWidget(const ScreenMap &mappings, const QString &deviceName, const ScreenRotation &rotation)
{
    Q_D(TabletAreaSelectionDialog);

    d->selectionWidget->setupWidget(mappings, deviceName, rotation);
}

void TabletAreaSelectionDialog::setupUi()
{
    Q_D(TabletAreaSelectionDialog);

    d->selectionWidget = new TabletAreaSelectionWidget(this);

    QVBoxLayout *layout = new QVBoxLayout;
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(d->selectionWidget);
    layout->addWidget(buttonBox);
    setLayout(layout);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    setWindowTitle(
        i18nc("Dialog title from a dialog which lets the user select an area of the tablet where the screen space will be mapped to.", "Select a Tablet Area"));
    setWindowIcon(QIcon::fromTheme(QLatin1String("preferences-desktop-tablet")));

    // connect( this, SIGNAL(okClicked()), this, SLOT(onOkClicked()) );
}

#include "moc_tabletareaselectiondialog.cpp"
