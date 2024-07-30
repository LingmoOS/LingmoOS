/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "PrinterBehavior.h"
#include "Debug.h"
#include "KCupsRequest.h"
#include "ui_PrinterBehavior.h"
#include <KCupsRequest.h>
#include <KLocalizedString>
#include <QPointer>

PrinterBehavior::PrinterBehavior(const QString &destName, bool isClass, QWidget *parent)
    : PrinterPage(parent)
    , ui(new Ui::PrinterBehavior)
    , m_destName(destName)
    , m_isClass(isClass)
{
    ui->setupUi(this);

    connect(ui->errorPolicyCB, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &PrinterBehavior::currentIndexChangedCB);
    connect(ui->operationPolicyCB, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &PrinterBehavior::currentIndexChangedCB);

    connect(ui->startingBannerCB, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &PrinterBehavior::currentIndexChangedCB);
    connect(ui->endingBannerCB, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &PrinterBehavior::currentIndexChangedCB);

    connect(ui->usersELB, &KEditListWidget::changed, this, &PrinterBehavior::userListChanged);
    connect(ui->allowRB, &QRadioButton::toggled, this, &PrinterBehavior::userListChanged);
}

PrinterBehavior::~PrinterBehavior()
{
    delete ui;
}

void PrinterBehavior::setValues(const KCupsPrinter &printer)
{
    int defaultChoice;
    ui->errorPolicyCB->clear();
    const QStringList errorPolicySupported = printer.errorPolicySupported();
    for (const QString &value : errorPolicySupported) {
        ui->errorPolicyCB->addItem(errorPolicyString(value), value);
    }
    const QStringList errorPolicy = printer.errorPolicy();
    if (!errorPolicy.isEmpty()) {
        defaultChoice = ui->errorPolicyCB->findData(errorPolicy.first());
        ui->errorPolicyCB->setCurrentIndex(defaultChoice);
        ui->errorPolicyCB->setProperty("defaultChoice", defaultChoice);
    }

    ui->operationPolicyCB->clear();
    const QStringList opPolicySupported = printer.opPolicySupported();
    for (const QString &value : opPolicySupported) {
        ui->operationPolicyCB->addItem(operationPolicyString(value), value);
    }
    const QStringList operationPolicy = printer.opPolicy();
    if (!errorPolicy.isEmpty()) {
        defaultChoice = ui->operationPolicyCB->findData(operationPolicy.first());
        ui->operationPolicyCB->setCurrentIndex(defaultChoice);
        ui->operationPolicyCB->setProperty("defaultChoice", defaultChoice);
    }

    ui->startingBannerCB->clear();
    ui->endingBannerCB->clear();
    const QStringList jobSheetsSupported = printer.jobSheetsSupported();
    for (const QString &value : jobSheetsSupported) {
        ui->startingBannerCB->addItem(jobSheetsString(value), value);
        ui->endingBannerCB->addItem(jobSheetsString(value), value);
    }
    const QStringList bannerPolicy = printer.jobSheetsDefault();
    if (bannerPolicy.size() == 2) {
        defaultChoice = ui->startingBannerCB->findData(bannerPolicy.at(0));
        ui->startingBannerCB->setCurrentIndex(defaultChoice);
        ui->startingBannerCB->setProperty("defaultChoice", defaultChoice);
        defaultChoice = ui->endingBannerCB->findData(bannerPolicy.at(1));
        ui->endingBannerCB->setCurrentIndex(defaultChoice);
        ui->endingBannerCB->setProperty("defaultChoice", defaultChoice);
    }

    if (!printer.requestingUserNameAllowed().isEmpty()) {
        QStringList list = printer.requestingUserNameAllowed();
        list.sort(); // sort the list here to be able to compare it later
        ui->usersELB->setEnabled(true);
        if (list != ui->usersELB->items()) {
            ui->usersELB->clear();
            ui->usersELB->insertStringList(list);
        }
        ui->usersELB->setProperty("defaultList", list);
        ui->allowRB->setProperty("defaultChoice", true);
        // Set checked AFTER the default choice was set
        // otherwise the signal will be emitted
        // which sets that we have a change
        ui->allowRB->setChecked(true);

    } else if (!printer.requestingUserNameDenied().isEmpty()) {
        QStringList list = printer.requestingUserNameDenied();
        list.sort(); // sort the list here to be able to compare it later
        ui->usersELB->setEnabled(true);
        if (list != ui->usersELB->items()) {
            ui->usersELB->clear();
            ui->usersELB->insertStringList(list);
        }
        ui->usersELB->setProperty("defaultList", list);
        ui->allowRB->setProperty("defaultChoice", false);
        // Set checked AFTER the default choice was set
        // otherwise the signal will be emitted
        // which sets that we have a change
        ui->preventRB->setChecked(true);
    }

    // Clear previous changes
    m_changes = 0;
    Q_EMIT changed(false);
    m_changedValues.clear();
    ui->errorPolicyCB->setProperty("different", false);
    ui->operationPolicyCB->setProperty("different", false);
    ui->startingBannerCB->setProperty("different", false);
    ui->endingBannerCB->setProperty("different", false);
    ui->usersELB->setProperty("different", false);
}

void PrinterBehavior::userListChanged()
{
    if (ui->usersELB->isEnabled() == false && (ui->allowRB->isChecked() || ui->preventRB->isChecked())) {
        // this only happen when the list was empty
        ui->usersELB->setEnabled(true);
    }

    QStringList currentList = ui->usersELB->items();
    // sort the list so we can be sure it's different
    currentList.sort();
    const QStringList defaultList = ui->usersELB->property("defaultList").toStringList();

    bool isDifferent = currentList != defaultList;
    if (isDifferent == false && currentList.isEmpty() == false) {
        // if the lists are equal and not empty the user might have
        // changed the Radio Button...
        if (ui->allowRB->isChecked() != ui->allowRB->property("defaultChoice").toBool()) {
            isDifferent = true;
        }
    }

    if (isDifferent != ui->usersELB->property("different").toBool()) {
        // it's different from the last time so add or remove changes
        isDifferent ? m_changes++ : m_changes--;

        ui->usersELB->setProperty("different", isDifferent);
        Q_EMIT changed(m_changes);
    }
}

void PrinterBehavior::currentIndexChangedCB(int index)
{
    auto comboBox = qobject_cast<QComboBox *>(sender());
    bool isDifferent = comboBox->property("defaultChoice").toInt() != index;
    qCDebug(PM_CONFIGURE_PRINTER) << Q_FUNC_INFO << "isDifferent" << isDifferent << this;

    if (isDifferent != comboBox->property("different").toBool()) {
        // it's different from the last time so add or remove changes
        isDifferent ? m_changes++ : m_changes--;

        comboBox->setProperty("different", isDifferent);
        qCDebug(PM_CONFIGURE_PRINTER) << Q_FUNC_INFO << m_changes << this;

        Q_EMIT changed(m_changes);
    }

    const QString attribute = comboBox->property("AttributeName").toString();
    QVariant value;
    // job-sheets-default has always two values
    if (attribute == QLatin1String("job-sheets-default")) {
        value = QStringList({ui->startingBannerCB->itemData(ui->startingBannerCB->currentIndex()).toString(),
                             ui->endingBannerCB->itemData(ui->endingBannerCB->currentIndex()).toString()});
    } else {
        value = comboBox->itemData(index).toString();
    }

    // store the new values
    if (isDifferent) {
        m_changedValues[attribute] = value;
    } else {
        m_changedValues.remove(attribute);
    }
}

QString PrinterBehavior::errorPolicyString(const QString &policy) const
{
    // TODO search for others policies of printer-error-policy-supported
    if (policy == QLatin1String("abort-job")) {
        return i18n("Abort job");
    } else if (policy == QLatin1String("retry-current-job")) {
        return i18n("Retry current job");
    } else if (policy == QLatin1String("retry-job")) {
        return i18n("Retry job");
    } else if (policy == QLatin1String("stop-printer")) {
        return i18n("Stop printer");
    }
    return policy;
}

QString PrinterBehavior::operationPolicyString(const QString &policy) const
{
    // TODO search for others policies of printer-error-policy-supported
    if (policy == QLatin1String("authenticated")) {
        return i18n("Authenticated");
    } else if (policy == QLatin1String("default")) {
        return i18n("Default");
    }
    return policy;
}

QString PrinterBehavior::jobSheetsString(const QString &policy) const
{
    // TODO search for others policies of printer-error-policy-supported
    if (policy == QLatin1String("none")) {
        return i18n("None");
    } else if (policy == QLatin1String("classified")) {
        return i18n("Classified");
    } else if (policy == QLatin1String("confidential")) {
        return i18n("Confidential");
    } else if (policy == QLatin1String("secret")) {
        return i18n("Secret");
    } else if (policy == QLatin1String("standard")) {
        return i18n("Standard");
    } else if (policy == QLatin1String("topsecret")) {
        return i18n("Topsecret");
    } else if (policy == QLatin1String("unclassified")) {
        return i18n("Unclassified");
    }
    return policy;
}

void PrinterBehavior::save()
{
    if (m_changes) {
        QVariantMap changedValues = m_changedValues;
        // since a QStringList might be big we get it here instead
        // of adding it at edit time.
        if (ui->usersELB->property("different").toBool()) {
            QStringList list = ui->usersELB->items();
            if (list.isEmpty()) {
                list << QLatin1String("all");
                changedValues[KCUPS_REQUESTING_USER_NAME_ALLOWED] = list;
            } else {
                if (ui->allowRB->isChecked()) {
                    changedValues[KCUPS_REQUESTING_USER_NAME_ALLOWED] = list;
                } else {
                    changedValues[KCUPS_REQUESTING_USER_NAME_DENIED] = list;
                }
            }
        }
        QPointer<KCupsRequest> request = new KCupsRequest;
        if (m_isClass) {
            request->addOrModifyClass(m_destName, changedValues);
        } else {
            request->addOrModifyPrinter(m_destName, changedValues);
        }
        request->waitTillFinished();
        if (request) {
            if (!request->hasError()) {
                request->getPrinterAttributes(m_destName, m_isClass, neededValues());
                request->waitTillFinished();
                if (request && !request->hasError() && !request->printers().isEmpty()) {
                    KCupsPrinter printer = request->printers().first();
                    setValues(printer);
                }
            }
            request->deleteLater();
        }
    }
}

void PrinterBehavior::setRemote(bool remote)
{
    ui->errorPolicyCB->setEnabled(!remote);
    ui->operationPolicyCB->setEnabled(!remote);
    ui->startingBannerCB->setEnabled(!remote);
    ui->endingBannerCB->setEnabled(!remote);
    ui->allowRB->setEnabled(!remote);
    ui->preventRB->setEnabled(!remote);
    ui->usersELB->setEnabled(!remote);
}

bool PrinterBehavior::hasChanges()
{
    return m_changes;
}

QStringList PrinterBehavior::neededValues() const
{
    return QStringList({KCUPS_JOB_SHEETS_DEFAULT,
                        KCUPS_JOB_SHEETS_SUPPORTED,

                        KCUPS_PRINTER_ERROR_POLICY,
                        KCUPS_PRINTER_ERROR_POLICY_SUPPORTED,

                        KCUPS_PRINTER_OP_POLICY,
                        KCUPS_PRINTER_OP_POLICY_SUPPORTED,

                        KCUPS_REQUESTING_USER_NAME_ALLOWED,
                        KCUPS_REQUESTING_USER_NAME_DENIED});
}

#include "moc_PrinterBehavior.cpp"
