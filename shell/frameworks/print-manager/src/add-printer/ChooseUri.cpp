/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ChooseUri.h"
#include "ui_ChooseUri.h"

#include <KCupsRequest.h>
#include <KLocalizedString>

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(PM_ADD_PRINTER)

ChooseUri::ChooseUri(QWidget *parent)
    : GenericPage(parent)
    , ui(new Ui::ChooseUri)
{
    ui->setupUi(this);

    ui->searchTB->setIcon(QIcon::fromTheme(QLatin1String("edit-find")));

    // setup default options
    setWindowTitle(i18nc("@title:window", "Select a Printer to Add"));

    connect(ui->addressLE, &QLineEdit::textChanged, this, &ChooseUri::textChanged);
    connect(ui->addressLE, &QLineEdit::returnPressed, this, &ChooseUri::findPrinters);
    connect(ui->searchTB, &QToolButton::clicked, this, &ChooseUri::findPrinters);
}

ChooseUri::~ChooseUri()
{
    delete ui;
}

void ChooseUri::setValues(const QVariantMap &args)
{
    m_args = args;
    bool visible = false;
    const QUrl url(args[KCUPS_DEVICE_URI].toString());
    if (url.url() == QLatin1String("other")) {
        ui->addressLE->clear();
        visible = true;
    } else if (url.scheme().isEmpty() && url.authority().isEmpty()) {
        ui->addressLE->setText(url.url() + QLatin1String("://"));
    } else {
        ui->addressLE->setText(url.url());
    }
    ui->searchTB->setVisible(visible);
    ui->addressLE->setFocus();
}

QVariantMap ChooseUri::values() const
{
    QVariantMap ret = m_args;

    ret[KCUPS_DEVICE_URI] = parsedURL(ui->addressLE->text()).toString();

    return ret;
}

bool ChooseUri::isValid() const
{
    const QString urlDefault = m_args[KCUPS_DEVICE_URI].toString();
    const QVariantMap args = values();
    const QString deviceUri = args[KCUPS_DEVICE_URI].toString();
    QUrl url(deviceUri);
    //    qCDebug(PM_ADD_PRINTER) << url << url.isValid() << url.isEmpty() << url.scheme().isEmpty() << url.host() << url.toString();
    return (url.isValid() && !url.isEmpty() && !url.scheme().isEmpty() && !url.host().isEmpty()) || urlDefault == deviceUri;
}

bool ChooseUri::canProceed() const
{
    return isValid();
}

void ChooseUri::load()
{
}

void ChooseUri::checkSelected()
{
    Q_EMIT allowProceed(isValid());
}

void ChooseUri::textChanged(const QString &text)
{
    QUrl url = parsedURL(text);

    if (url.isValid()
        && (url.scheme().isEmpty() || url.scheme() == QLatin1String("http") || url.scheme() == QLatin1String("https")
            || url.scheme() == QLatin1String("ipp"))) {
        // TODO maybe cups library can connect to more protocols
        ui->searchTB->setEnabled(true);
    } else {
        ui->searchTB->setEnabled(false);
    }

    checkSelected();
}

void ChooseUri::findPrinters()
{
    const QUrl url = parsedURL(ui->addressLE->text());

    auto conn = new KCupsConnection(url, this);
    auto request = new KCupsRequest(conn);
    connect(request, &KCupsRequest::finished, this, &ChooseUri::getPrintersFinished);

    request->setProperty("URI", url);

    Q_EMIT startWorking();
    request->getPrinters({KCUPS_PRINTER_NAME,
                          KCUPS_PRINTER_STATE,
                          KCUPS_PRINTER_IS_SHARED,
                          KCUPS_PRINTER_IS_ACCEPTING_JOBS,
                          KCUPS_PRINTER_TYPE,
                          KCUPS_PRINTER_LOCATION,
                          KCUPS_PRINTER_INFO,
                          KCUPS_PRINTER_MAKE_AND_MODEL});
}

void ChooseUri::getPrintersFinished(KCupsRequest *request)
{
    QUrl uri = request->property("URI").value<QUrl>();
    QUrl url;
    url.setScheme(QStringLiteral("ipp"));
    url.setAuthority(uri.authority());

    KCupsPrinters printers = request->printers();
    if (request->hasError()) {
        Q_EMIT errorMessage(request->errorMsg());
    } else {
        Q_EMIT insertDevice(QLatin1String("network"), url.authority(), url.authority(), QString(), url.url(), QString(), printers);
    }

    request->deleteLater();
    request->connection()->deleteLater();
    Q_EMIT stopWorking();
}

QUrl ChooseUri::parsedURL(const QString &text) const
{
    const QString urlDefault = m_args[KCUPS_DEVICE_URI].toString();
    QUrl url(QUrl::fromUserInput(text));
    if (url.host().isEmpty() && !text.contains(QLatin1String("://")) && urlDefault != text) {
        url = QUrl();
        // URI might be scsi, network on anything that didn't match before
        if (m_args[KCUPS_DEVICE_URI].toString() != QLatin1String("other")) {
            url.setScheme(m_args[KCUPS_DEVICE_URI].toString());
        }
        url.setAuthority(text);
    }
    return url;
}

#include "moc_ChooseUri.cpp"
