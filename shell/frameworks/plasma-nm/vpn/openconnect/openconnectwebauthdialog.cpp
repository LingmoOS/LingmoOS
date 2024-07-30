/*
    SPDX-FileCopyrightText: 2024 Joel Holdsworth <joel@airwebreathe.org.uk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "openconnectwebauthdialog.h"

#include <QButtonGroup>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWebEngineView>

class OpenconnectWebAuthDialogPrivate
{
public:
    QWebEngineWebAuthUxRequest *uxRequest;
    QButtonGroup *buttonGroup = nullptr;
    QScrollArea *scrollArea = nullptr;
    QWidget *selectAccountWidget = nullptr;
    QVBoxLayout *selectAccountLayout = nullptr;

    Ui::OpenconnectWebAuthDialog uiDialog;
};

OpenconnectWebAuthDialog::OpenconnectWebAuthDialog(QWebEngineWebAuthUxRequest *request, QWidget *parent)
    : QDialog(parent)
    , d_ptr(new OpenconnectWebAuthDialogPrivate)
{
    Q_D(OpenconnectWebAuthDialog);

    d->uxRequest = request;

    d->uiDialog.setupUi(this);

    d->buttonGroup = new QButtonGroup(this);
    d->buttonGroup->setExclusive(true);

    d->scrollArea = new QScrollArea(this);
    d->selectAccountWidget = new QWidget(this);
    d->scrollArea->setWidget(d->selectAccountWidget);
    d->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    d->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    d->selectAccountWidget->resize(400, 150);
    d->selectAccountLayout = new QVBoxLayout(d->selectAccountWidget);
    d->uiDialog.mainVerticalLayout->addWidget(d->scrollArea);
    d->selectAccountLayout->setAlignment(Qt::AlignTop);

    updateDisplay();

    connect(d->uiDialog.buttonBox, &QDialogButtonBox::rejected, this, qOverload<>(&OpenconnectWebAuthDialog::onCancelRequest));

    connect(d->uiDialog.buttonBox, &QDialogButtonBox::accepted, this, qOverload<>(&OpenconnectWebAuthDialog::onAcceptRequest));

    QAbstractButton *button = d->uiDialog.buttonBox->button(QDialogButtonBox::Retry);
    connect(button, &QAbstractButton::clicked, this, qOverload<>(&OpenconnectWebAuthDialog::onRetry));

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
}

OpenconnectWebAuthDialog::~OpenconnectWebAuthDialog()
{
    Q_D(OpenconnectWebAuthDialog);

    QList<QAbstractButton *> buttons = d->buttonGroup->buttons();
    for (auto itr = buttons.begin(); itr != buttons.end(); itr++) {
        QAbstractButton *radioButton = *itr;
        delete radioButton;
    }

    if (d->buttonGroup)
        delete d->buttonGroup;

    // selectAccountWidget and it's children will get deleted when scroll area is destroyed
    if (d->scrollArea)
        delete d->scrollArea;
}

void OpenconnectWebAuthDialog::updateDisplay()
{
    Q_D(OpenconnectWebAuthDialog);

    switch (d->uxRequest->state()) {
    case QWebEngineWebAuthUxRequest::WebAuthUxState::SelectAccount:
        setupSelectAccountUI();
        break;
    case QWebEngineWebAuthUxRequest::WebAuthUxState::CollectPin:
        setupCollectPinUI();
        break;
    case QWebEngineWebAuthUxRequest::WebAuthUxState::FinishTokenCollection:
        setupFinishCollectTokenUI();
        break;
    case QWebEngineWebAuthUxRequest::WebAuthUxState::RequestFailed:
        setupErrorUI();
        break;
    default:
        break;
    }

    adjustSize();
}

void OpenconnectWebAuthDialog::setupSelectAccountUI()
{
    Q_D(OpenconnectWebAuthDialog);

    d->uiDialog.headingLabel->setText(i18n("Choose a Passkey"));
    d->uiDialog.description->setText(i18n("Which passkey do you want to use for %1?", d->uxRequest->relyingPartyId()));
    d->uiDialog.pinGroupBox->setVisible(false);
    d->uiDialog.mainVerticalLayout->removeWidget(d->uiDialog.pinGroupBox);
    d->uiDialog.buttonBox->button(QDialogButtonBox::Retry)->setVisible(false);

    clearSelectAccountButtons();
    d->scrollArea->setVisible(true);
    d->selectAccountWidget->resize(this->width(), this->height());
    QStringList userNames = d->uxRequest->userNames();

    // Create radio buttons for each name
    for (const QString &name : userNames) {
        QRadioButton *radioButton = new QRadioButton(name);
        d->selectAccountLayout->addWidget(radioButton);
        d->buttonGroup->addButton(radioButton);
    }

    d->uiDialog.buttonBox->button(QDialogButtonBox::Ok)->setText(i18n("Ok"));
    d->uiDialog.buttonBox->button(QDialogButtonBox::Ok)->setVisible(true);
    d->uiDialog.buttonBox->button(QDialogButtonBox::Cancel)->setVisible(true);
    d->uiDialog.buttonBox->button(QDialogButtonBox::Retry)->setVisible(false);
}

void OpenconnectWebAuthDialog::setupFinishCollectTokenUI()
{
    Q_D(OpenconnectWebAuthDialog);

    clearSelectAccountButtons();
    d->uiDialog.headingLabel->setText(i18n("Use your security key with %1", d->uxRequest->relyingPartyId()));
    d->uiDialog.description->setText(i18n("Touch your security key again to complete the request."));
    d->uiDialog.pinGroupBox->setVisible(false);
    d->uiDialog.buttonBox->button(QDialogButtonBox::Ok)->setVisible(false);
    d->uiDialog.buttonBox->button(QDialogButtonBox::Retry)->setVisible(false);
    d->scrollArea->setVisible(false);
}

void OpenconnectWebAuthDialog::setupCollectPinUI()
{
    Q_D(OpenconnectWebAuthDialog);

    clearSelectAccountButtons();

    d->uiDialog.mainVerticalLayout->addWidget(d->uiDialog.pinGroupBox);
    d->uiDialog.pinGroupBox->setVisible(true);
    d->uiDialog.confirmPinLabel->setVisible(false);
    d->uiDialog.confirmPinLineEdit->setVisible(false);
    d->uiDialog.buttonBox->button(QDialogButtonBox::Ok)->setText(i18n("Next"));
    d->uiDialog.buttonBox->button(QDialogButtonBox::Ok)->setVisible(true);
    d->uiDialog.buttonBox->button(QDialogButtonBox::Cancel)->setVisible(true);
    d->uiDialog.buttonBox->button(QDialogButtonBox::Retry)->setVisible(false);
    d->scrollArea->setVisible(false);

    QWebEngineWebAuthPinRequest pinRequestInfo = d->uxRequest->pinRequest();

    if (pinRequestInfo.reason == QWebEngineWebAuthUxRequest::PinEntryReason::Challenge) {
        d->uiDialog.headingLabel->setText(i18n("PIN Required"));
        d->uiDialog.description->setText(i18n("Enter the PIN for your security key"));
        d->uiDialog.confirmPinLabel->setVisible(false);
        d->uiDialog.confirmPinLineEdit->setVisible(false);
    } else {
        if (pinRequestInfo.reason == QWebEngineWebAuthUxRequest::PinEntryReason::Set) {
            d->uiDialog.headingLabel->setText(i18n("New PIN Required"));
            d->uiDialog.description->setText(i18n("Set new PIN for your security key"));
        } else {
            d->uiDialog.headingLabel->setText(i18n("Change PIN Required"));
            d->uiDialog.description->setText(i18n("Change PIN for your security key"));
        }
        d->uiDialog.confirmPinLabel->setVisible(true);
        d->uiDialog.confirmPinLineEdit->setVisible(true);
    }

    QString errorDetails;
    switch (pinRequestInfo.error) {
    case QWebEngineWebAuthUxRequest::PinEntryError::NoError:
        break;
    case QWebEngineWebAuthUxRequest::PinEntryError::InternalUvLocked:
        errorDetails = i18n("Internal User Verification Locked");
        break;
    case QWebEngineWebAuthUxRequest::PinEntryError::WrongPin:
        errorDetails = i18n("Wrong PIN. %1 attempts remaining.", pinRequestInfo.remainingAttempts);
        break;
    case QWebEngineWebAuthUxRequest::PinEntryError::TooShort:
        errorDetails = i18n("Too Short. %1 attempts remaining.", pinRequestInfo.remainingAttempts);
        break;
    case QWebEngineWebAuthUxRequest::PinEntryError::InvalidCharacters:
        errorDetails = i18n("Invalid Characters. %1 attempts remaining.", pinRequestInfo.remainingAttempts);
        ;
        break;
    case QWebEngineWebAuthUxRequest::PinEntryError::SameAsCurrentPin:
        errorDetails = i18n("Same as current PIN. %1 attempts remaining.", pinRequestInfo.remainingAttempts);
        break;
    }

    d->uiDialog.pinEntryErrorLabel->setText(errorDetails);
}

void OpenconnectWebAuthDialog::onCancelRequest()
{
    Q_D(OpenconnectWebAuthDialog);

    d->uxRequest->cancel();
}

void OpenconnectWebAuthDialog::onAcceptRequest()
{
    Q_D(OpenconnectWebAuthDialog);

    switch (d->uxRequest->state()) {
    case QWebEngineWebAuthUxRequest::WebAuthUxState::SelectAccount:
        if (d->buttonGroup->checkedButton()) {
            d->uxRequest->setSelectedAccount(d->buttonGroup->checkedButton()->text());
        }
        break;
    case QWebEngineWebAuthUxRequest::WebAuthUxState::CollectPin:
        d->uxRequest->setPin(d->uiDialog.pinLineEdit->text());
        break;
    default:
        break;
    }
}

void OpenconnectWebAuthDialog::setupErrorUI()
{
    Q_D(OpenconnectWebAuthDialog);

    clearSelectAccountButtons();
    QString errorDescription;
    QString errorHeading = i18n("Something went wrong");
    bool isVisibleRetry = false;
    switch (d->uxRequest->requestFailureReason()) {
    case QWebEngineWebAuthUxRequest::RequestFailureReason::Timeout:
        errorDescription = i18n("Request Timeout");
        break;
    case QWebEngineWebAuthUxRequest::RequestFailureReason::KeyNotRegistered:
        errorDescription = i18n("Key not registered");
        break;
    case QWebEngineWebAuthUxRequest::RequestFailureReason::KeyAlreadyRegistered:
        errorDescription = i18n(
            "You already registered this device."
            "Try again with device");
        isVisibleRetry = true;
        break;
    case QWebEngineWebAuthUxRequest::RequestFailureReason::SoftPinBlock:
        errorDescription = i18n(
            "The security key is locked because the wrong PIN was entered too many times."
            "To unlock it, remove and reinsert it.");
        isVisibleRetry = true;
        break;
    case QWebEngineWebAuthUxRequest::RequestFailureReason::HardPinBlock:
        errorDescription = i18n(
            "The security key is locked because the wrong PIN was entered too many times."
            " You'll need to reset the security key.");
        break;
    case QWebEngineWebAuthUxRequest::RequestFailureReason::AuthenticatorRemovedDuringPinEntry:
        errorDescription = i18n("Authenticator removed during verification. Please reinsert and try again");
        break;
    case QWebEngineWebAuthUxRequest::RequestFailureReason::AuthenticatorMissingResidentKeys:
        errorDescription = i18n("Authenticator doesn't have resident key support");
        break;
    case QWebEngineWebAuthUxRequest::RequestFailureReason::AuthenticatorMissingUserVerification:
        errorDescription = i18n("Authenticator missing user verification");
        break;
    case QWebEngineWebAuthUxRequest::RequestFailureReason::AuthenticatorMissingLargeBlob:
        errorDescription = i18n("Authenticator missing Large Blob support");
        break;
    case QWebEngineWebAuthUxRequest::RequestFailureReason::NoCommonAlgorithms:
        errorDescription = i18n("No common algorithms");
        break;
    case QWebEngineWebAuthUxRequest::RequestFailureReason::StorageFull:
        errorDescription = i18n("Storage Full");
        break;
    case QWebEngineWebAuthUxRequest::RequestFailureReason::UserConsentDenied:
        errorDescription = i18n("User consent denied");
        break;
    case QWebEngineWebAuthUxRequest::RequestFailureReason::WinUserCancelled:
        errorDescription = i18n("User Cancelled Request");
        break;
    }

    d->uiDialog.headingLabel->setText(errorHeading);
    d->uiDialog.description->setText(errorDescription);
    d->uiDialog.description->adjustSize();
    d->uiDialog.pinGroupBox->setVisible(false);
    d->uiDialog.buttonBox->button(QDialogButtonBox::Ok)->setVisible(false);
    d->uiDialog.buttonBox->button(QDialogButtonBox::Retry)->setVisible(isVisibleRetry);
    if (isVisibleRetry)
        d->uiDialog.buttonBox->button(QDialogButtonBox::Retry)->setFocus();
    d->uiDialog.buttonBox->button(QDialogButtonBox::Cancel)->setVisible(true);
    d->uiDialog.buttonBox->button(QDialogButtonBox::Cancel)->setText(i18n("Close"));
    d->scrollArea->setVisible(false);
}

void OpenconnectWebAuthDialog::onRetry()
{
    Q_D(OpenconnectWebAuthDialog);

    d->uxRequest->retry();
}

void OpenconnectWebAuthDialog::clearSelectAccountButtons()
{
    Q_D(OpenconnectWebAuthDialog);

    QList<QAbstractButton *> buttons = d->buttonGroup->buttons();
    for (auto itr = buttons.begin(); itr != buttons.end(); itr++) {
        QAbstractButton *radioButton = *itr;
        d->selectAccountLayout->removeWidget(radioButton);
        d->buttonGroup->removeButton(radioButton);
        delete radioButton;
    }
}
