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

#include "generalpagewidget.h"
#include "ui_generalpagewidget.h"

#include "profilemanagement.h"

// common
#include "dbustabletinterface.h"
#include "globalactions.h"
#include "tabletinfo.h"

// stdlib
#include <memory>

// KDE includes
#include <KShortcutsEditor>

// Qt includes
#include <QDBusInterface>
#include <QDBusReply>
#include <QInputDialog>
#include <QListWidget>
#include <QListWidgetItem>
#include <QStringList>

using namespace Wacom;

GeneralPageWidget::GeneralPageWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GeneralPageWidget)
{
    ui->setupUi(this);

    // if someone adds another action also add it to kded/tabletdeamon.cpp
    _actionCollection = new GlobalActions(true, this);

    _shortcutEditor = new KShortcutsEditor(this, KShortcutsEditor::GlobalAction);
    _shortcutEditor->addCollection(_actionCollection, i18n("Wacom Tablet Settings"));

    ui->shortcutGroupBox->layout()->addWidget(_shortcutEditor);

    // setup icons in the profilerotation list box
    ui->pbAddToRotationList->setIcon(QIcon::fromTheme(QLatin1String("list-add")));
    ui->pbRemoveFromRotationList->setIcon(QIcon::fromTheme(QLatin1String("list-remove")));
    ui->pbUp->setIcon(QIcon::fromTheme(QLatin1String("arrow-up")));
    ui->pbDown->setIcon(QIcon::fromTheme(QLatin1String("arrow-down")));

    connect(_shortcutEditor, SIGNAL(keyChange()), this, SLOT(profileChanged()));
}

GeneralPageWidget::~GeneralPageWidget()
{
    delete ui;
    delete _actionCollection;
    delete _shortcutEditor;
}

void GeneralPageWidget::setTabletId(const QString &tabletId)
{
    _tabletId = tabletId;
}

void GeneralPageWidget::saveToProfile()
{
    QList<QListWidgetItem *> items = ui->lwRotationList->findItems(QLatin1String("*"), Qt::MatchWrap | Qt::MatchWildcard);
    QStringList newRotationList;
    foreach (QListWidgetItem *item, items) {
        newRotationList.append(item->text());
    }

    DBusTabletInterface::instance().setProfileRotationList(_tabletId, newRotationList);

    _shortcutEditor->save();
}

void GeneralPageWidget::loadFromProfile()
{
}

void GeneralPageWidget::profileChanged()
{
    emit changed();
}

void GeneralPageWidget::reloadWidget()
{
    // get information via DBus
    QDBusReply<QString> deviceName = DBusTabletInterface::instance().getInformation(_tabletId, TabletInfo::TabletName.key());

    // load rotation profile list based on current tablet
    QDBusReply<QStringList> rotationList = DBusTabletInterface::instance().getProfileRotationList(_tabletId);
    ui->lwRotationList->clear();
    ui->lwRotationList->addItems(rotationList);
}

void GeneralPageWidget::profileUp()
{
    QListWidgetItem *curItem = ui->lwRotationList->currentItem();
    if (curItem) {
        int curRow = ui->lwRotationList->row(curItem);
        int nextRow = curRow - 1;
        if (nextRow >= 0) {
            QListWidgetItem *curItem = ui->lwRotationList->takeItem(curRow);
            ui->lwRotationList->insertItem(nextRow, curItem->text());
            ui->lwRotationList->setCurrentRow(nextRow);
        }
        emit changed();
    }
}

void GeneralPageWidget::profileDown()
{
    QListWidgetItem *curItem = ui->lwRotationList->currentItem();
    if (curItem) {
        int curRow = ui->lwRotationList->row(curItem);
        curItem = ui->lwRotationList->takeItem(curRow);
        ui->lwRotationList->insertItem(curRow + 1, curItem->text());
        ui->lwRotationList->setCurrentRow(curRow + 1);

        emit changed();
    }
}

void GeneralPageWidget::profileAdd()
{
    bool ok = false;
    QString item = QInputDialog::getItem(this,
                                         i18n("Profile List"),
                                         i18n("Select the Profile you want to add:"),
                                         ProfileManagement::instance().availableProfiles(),
                                         0,
                                         false,
                                         &ok);
    if (ok && !item.isEmpty()) {
        ui->lwRotationList->addItem(item);

        emit changed();
    }
}

void GeneralPageWidget::profileRemove()
{
    QListWidgetItem *curItem = ui->lwRotationList->currentItem();
    if (curItem) {
        ui->lwRotationList->removeItemWidget(curItem);
        delete curItem;

        emit changed();
    }
}

#include "moc_generalpagewidget.cpp"
