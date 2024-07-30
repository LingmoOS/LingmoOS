/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2003 Waldo Bastian <bastian@kde.org>
   SPDX-FileCopyrightText: 2003 David Faure <faure@kde.org>
   SPDX-FileCopyrightText: 2002 Daniel Molkentin <molkentin@kde.org>

   SPDX-License-Identifier: GPL-2.0-only
*/

// Own
#include "kservicelistwidget.h"

// Qt
#include <QHBoxLayout>
#include <QPushButton>
#include <QStandardPaths>
#include <QVBoxLayout>

// KDE
#include <KLocalizedString>
#include <KMessageBox>
#include <KOpenWithDialog>
#include <KPropertiesDialog>

// Local
#include "kserviceselectdlg.h"
#include "mimetypedata.h"

KServiceListItem::KServiceListItem(const KService::Ptr &pService)
    : QListWidgetItem()
    , storageId(pService->storageId())
    , desktopPath(pService->entryPath())
{
    setText(pService->name());
    setIcon(QIcon::fromTheme(pService->icon()));

    if (!pService->isApplication()) {
        localPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/kservices6/") + desktopPath;
    } else {
        localPath = pService->locateLocal();
    }
}

PluginListItem::PluginListItem(const KPluginMetaData &data)
    : QListWidgetItem()
    , metaData(data)
{
    setText(i18n("%1 (%2)", metaData.name(), metaData.pluginId()));
    setIcon(QIcon::fromTheme(metaData.iconName()));
}

KServiceListWidget::KServiceListWidget(Kind kind, QWidget *parent)
    : QGroupBox(kind == SERVICELIST_APPLICATIONS ? i18n("Application Preference Order") : i18n("Services Preference Order"), parent)
    , m_kind(kind)
    , m_mimeTypeData(nullptr)
    , m_allowMultiApply(false)
{
    QHBoxLayout *lay = new QHBoxLayout(this);

    servicesLB = new QListWidget(this);
    connect(servicesLB, &QListWidget::itemSelectionChanged, this, &KServiceListWidget::enableMoveButtons);
    lay->addWidget(servicesLB);
    connect(servicesLB, &QListWidget::itemDoubleClicked, this, &KServiceListWidget::editService);

    QString wtstr = (kind == SERVICELIST_APPLICATIONS ? i18n("This is a list of applications associated with files of the selected"
                                                             " file type. This list is shown in Konqueror's context menus when you select"
                                                             " \"Open With...\". If more than one application is associated with this file type,"
                                                             " then the list is ordered by priority with the uppermost item taking precedence"
                                                             " over the others.")
                                                      : i18n("This is a list of services associated with files of the selected"
                                                             " file type. This list is shown in Konqueror's context menus when you select"
                                                             " a \"Preview with...\" option. If more than one service is associated with this file type,"
                                                             " then the list is ordered by priority with the uppermost item taking precedence"
                                                             " over the others."));

    setWhatsThis(wtstr);
    servicesLB->setWhatsThis(wtstr);

    QVBoxLayout *btnsLay = new QVBoxLayout();
    lay->addLayout(btnsLay);

    servUpButton = new QPushButton(i18n("Move &Up"), this);
    servUpButton->setIcon(QIcon::fromTheme(QStringLiteral("arrow-up")));
    servUpButton->setEnabled(false);
    connect(servUpButton, &QAbstractButton::clicked, this, &KServiceListWidget::promoteService);
    btnsLay->addWidget(servUpButton);

    servUpButton->setWhatsThis(kind == SERVICELIST_APPLICATIONS ? i18n("Assigns a higher priority to the selected\n"
                                                                       "application, moving it up in the list. Note:  This\n"
                                                                       "only affects the selected application if the file type is\n"
                                                                       "associated with more than one application.")
                                                                : i18n("Assigns a higher priority to the selected\n"
                                                                       "service, moving it up in the list."));

    servDownButton = new QPushButton(i18n("Move &Down"), this);
    servDownButton->setIcon(QIcon::fromTheme(QStringLiteral("arrow-down")));
    servDownButton->setEnabled(false);
    connect(servDownButton, &QAbstractButton::clicked, this, &KServiceListWidget::demoteService);
    btnsLay->addWidget(servDownButton);
    servDownButton->setWhatsThis(kind == SERVICELIST_APPLICATIONS ? i18n("Assigns a lower priority to the selected\n"
                                                                         "application, moving it down in the list. Note: This \n"
                                                                         "only affects the selected application if the file type is\n"
                                                                         "associated with more than one application.")
                                                                  : i18n("Assigns a lower priority to the selected\n"
                                                                         "service, moving it down in the list."));

    servNewButton = new QPushButton(i18n("Add..."), this);
    servNewButton->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    servNewButton->setEnabled(false);
    connect(servNewButton, &QAbstractButton::clicked, this, &KServiceListWidget::addService);
    btnsLay->addWidget(servNewButton);
    servNewButton->setWhatsThis(i18n("Add a new application for this file type."));

    servEditButton = new QPushButton(i18n("Edit..."), this);
    servEditButton->setIcon(QIcon::fromTheme(QStringLiteral("edit-rename")));
    servEditButton->setEnabled(false);
    connect(servEditButton, &QAbstractButton::clicked, this, &KServiceListWidget::editService);
    btnsLay->addWidget(servEditButton);
    servEditButton->setWhatsThis(i18n("Edit command line of the selected application."));

    servRemoveButton = new QPushButton(i18n("Remove"), this);
    servRemoveButton->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
    servRemoveButton->setEnabled(false);
    connect(servRemoveButton, &QAbstractButton::clicked, this, &KServiceListWidget::removeService);
    btnsLay->addWidget(servRemoveButton);
    servRemoveButton->setWhatsThis(i18n("Remove the selected application from the list."));

    servApplyToButton = new QPushButton(i18n("Apply To..."), this);
    servApplyToButton->setIcon(QIcon::fromTheme(QStringLiteral("edit-copy")));
    servApplyToButton->setEnabled(false);
    connect(servApplyToButton, &QAbstractButton::clicked, this, &KServiceListWidget::applyTo);
    btnsLay->addWidget(servApplyToButton);
    servApplyToButton->setWhatsThis(i18n("Apply the current preference order to other file types."));

    btnsLay->addStretch(1);
}

void KServiceListWidget::setMimeTypeData(MimeTypeData *mimeTypeData)
{
    m_mimeTypeData = mimeTypeData;
    if (servNewButton) {
        servNewButton->setEnabled(true);
    }
    // will need a selection
    servUpButton->setEnabled(false);
    servDownButton->setEnabled(false);

    servicesLB->clear();
    servicesLB->setEnabled(false);

    if (m_mimeTypeData) {
        if (m_kind == SERVICELIST_APPLICATIONS) {
            const QStringList services = m_mimeTypeData->appServices();
            if (services.isEmpty()) {
                if (m_kind == SERVICELIST_APPLICATIONS) {
                    servicesLB->addItem(i18nc("No applications associated with this file type", "None"));
                }
            } else {
                for (const QString &service : services) {
                    if (KService::Ptr pService = KService::serviceByStorageId(service)) {
                        servicesLB->addItem(new KServiceListItem(pService));
                    }
                }
                servicesLB->setEnabled(true);
            }
        } else {
            const QStringList parts = m_mimeTypeData->embedParts();
            if (parts.isEmpty()) {
                servicesLB->addItem(new QListWidgetItem(i18nc("No components associated with this file type", "None"), nullptr, -1));
            } else {
                servicesLB->setEnabled(true);
                for (const QString &partId : parts) {
                    if (KPluginMetaData data(QStringLiteral("kf6/parts/") + partId); data.isValid()) {
                        servicesLB->addItem(new PluginListItem(data));
                    }
                }
            }
        }
    }

    if (servRemoveButton) {
        servRemoveButton->setEnabled(servicesLB->currentRow() > -1);
    }
    if (servEditButton) {
        servEditButton->setEnabled(servicesLB->currentRow() > -1);
    }
    if (servApplyToButton) {
        servApplyToButton->setEnabled(m_allowMultiApply);
    }
}

void KServiceListWidget::allowMultiApply(bool allow)
{
    m_allowMultiApply = allow;

    if (m_mimeTypeData && servApplyToButton) {
        servApplyToButton->setEnabled(m_allowMultiApply);
    }
}

void KServiceListWidget::promoteService()
{
    if (!servicesLB->isEnabled()) {
        return;
    }

    int selIndex = servicesLB->currentRow();
    if (selIndex == 0) {
        return;
    }

    QListWidgetItem *selItem = servicesLB->item(selIndex);
    servicesLB->takeItem(selIndex);
    servicesLB->insertItem(selIndex - 1, selItem);
    servicesLB->setCurrentRow(selIndex - 1);

    updatePreferredServices();

    Q_EMIT changed(true);
}

void KServiceListWidget::demoteService()
{
    if (!servicesLB->isEnabled()) {
        return;
    }

    int selIndex = servicesLB->currentRow();
    if (selIndex == servicesLB->count() - 1) {
        return;
    }

    QListWidgetItem *selItem = servicesLB->item(selIndex);
    servicesLB->takeItem(selIndex);
    servicesLB->insertItem(selIndex + 1, selItem);
    servicesLB->setCurrentRow(selIndex + 1);

    updatePreferredServices();

    Q_EMIT changed(true);
}

void KServiceListWidget::addService()
{
    if (!m_mimeTypeData) {
        return;
    }

    if (m_kind == SERVICELIST_APPLICATIONS) {
        KOpenWithDialog dlg(m_mimeTypeData->name(), QString(), this);
        dlg.setSaveNewApplications(true);
        if (dlg.exec() != QDialog::Accepted) {
            return;
        }

        KService::Ptr service = dlg.service();

        Q_ASSERT(service);
        if (!service) {
            return; // Don't crash if KOpenWith wasn't able to create service.
        }
        if (m_mimeTypeData->appServices().contains(service->storageId())) {
            return;
        }
        servicesLB->insertItem(0, new KServiceListItem(service));
    } else {
        KPartSelectDlg dlg(this);
        if (dlg.exec() != QDialog::Accepted) {
            return;
        }
        const bool valid = dlg.chosenPart().isValid();
        Q_ASSERT(valid);
        if (m_mimeTypeData->embedParts().contains(dlg.chosenPart().pluginId())) {
            return;
        }
        auto item = new PluginListItem(dlg.chosenPart());
        servicesLB->insertItem(0, item);
    }

    // We inserted a new service at the beginning, if we had a None entry before, it must
    // be the second on in the element. Check the type to be sure of it
    if (servicesLB->count() > 0 && servicesLB->item(1)->type() == -1) {
        delete servicesLB->takeItem(1);
        servicesLB->setEnabled(true);
    }

    servicesLB->setCurrentItem(nullptr);
    updatePreferredServices();
    Q_EMIT changed(true);
}

void KServiceListWidget::editService()
{
    if (!m_mimeTypeData) {
        return;
    }
    const int selected = servicesLB->currentRow();
    if (selected < 0) {
        return;
    }

    // Only edit applications, not services as
    // they don't have any parameters
    if (m_kind != SERVICELIST_APPLICATIONS) {
        return;
    }

    // Just like popping up an add dialog except that we
    // pass the current command line as a default
    KServiceListItem *selItem = (KServiceListItem *)servicesLB->item(selected);
    const QString desktopPath = selItem->desktopPath;

    KService::Ptr service = KService::serviceByDesktopPath(desktopPath);
    if (!service) {
        return;
    }

    QString path = service->entryPath();
    {
        // If the path to the desktop file is relative, try to get the full
        // path from QStandardPaths.
        QString fullPath = QStandardPaths::locate(QStandardPaths::ApplicationsLocation, path);
        if (!fullPath.isEmpty()) {
            path = fullPath;
        }
    }

    KFileItem item(QUrl::fromLocalFile(path), QStringLiteral("application/x-desktop"), KFileItem::Unknown);
    KPropertiesDialog dlg(item, this);
    if (dlg.exec() != QDialog::Accepted) {
        return;
    }

    // Note that at this point, ksycoca has been updated,
    // and setMimeTypeData has been called again, so all the items have been recreated.

    // Reload service
    service = KService::serviceByDesktopPath(desktopPath);
    if (!service) {
        return;
    }

    // Remove the old one...
    delete servicesLB->takeItem(selected);

    // ...check that it's not a duplicate entry...
    bool addIt = true;
    for (int index = 0; index < servicesLB->count(); index++) {
        if (static_cast<KServiceListItem *>(servicesLB->item(index))->desktopPath == service->entryPath()) {
            addIt = false;
            break;
        }
    }

    // ...and add it in the same place as the old one:
    if (addIt) {
        servicesLB->insertItem(selected, new KServiceListItem(service));
        servicesLB->setCurrentRow(selected);
    }

    updatePreferredServices();

    Q_EMIT changed(true);
}

void KServiceListWidget::removeService()
{
    if (!m_mimeTypeData) {
        return;
    }

    int selected = servicesLB->currentRow();

    if (selected >= 0) {
        delete servicesLB->takeItem(selected);
        updatePreferredServices();

        Q_EMIT changed(true);
    }

    // Update buttons and service list again (e.g. to re-add "None")
    setMimeTypeData(m_mimeTypeData);
}

void KServiceListWidget::updatePreferredServices()
{
    if (!m_mimeTypeData) {
        return;
    }
    QStringList sl;
    unsigned int count = servicesLB->count();
    for (unsigned int i = 0; i < count; i++) {
        const auto &item = servicesLB->item(i);
        Q_ASSERT(item->type() != -1);
        if (m_kind == SERVICELIST_APPLICATIONS) {
            auto sli = dynamic_cast<KServiceListItem *>(item);
            if (!sli) { // Is a textual item like the default 'None' entry
                continue;
            }
            sl.append(sli->storageId);
        } else {
            auto pluginItem = dynamic_cast<PluginListItem *>(item);
            if (!pluginItem) { // Is a textual item like the default 'None' entry
                continue;
            }
            sl.append(pluginItem->metaData.pluginId());
        }
    }
    sl.removeDuplicates();
    if (m_kind == SERVICELIST_APPLICATIONS) {
        m_mimeTypeData->setAppServices(sl);
    } else {
        m_mimeTypeData->setEmbedParts(sl);
    }
}

void KServiceListWidget::applyTo()
{
    Q_EMIT multiApply(m_kind);
}

void KServiceListWidget::enableMoveButtons()
{
    int idx = servicesLB->currentRow();
    if (servicesLB->model()->rowCount() <= 1) {
        servUpButton->setEnabled(false);
        servDownButton->setEnabled(false);
    } else if (idx == (servicesLB->model()->rowCount() - 1)) {
        servUpButton->setEnabled(true);
        servDownButton->setEnabled(false);
    } else if (idx == 0) {
        servUpButton->setEnabled(false);
        servDownButton->setEnabled(true);
    } else {
        servUpButton->setEnabled(true);
        servDownButton->setEnabled(true);
    }

    if (servRemoveButton) {
        servRemoveButton->setEnabled(true);
    }

    if (servEditButton) {
        servEditButton->setEnabled(m_kind == SERVICELIST_APPLICATIONS);
    }
}

#include "moc_kservicelistwidget.cpp"
