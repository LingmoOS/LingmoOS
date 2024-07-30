/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "PageAddPrinter.h"
#include "ui_PageAddPrinter.h"

#include <kde-add-printer_log.h>

#include <KCupsRequest.h>
#include <KLocalizedString>

#include <QPainter>
#include <QPointer>
#include <QRegularExpressionValidator>

PageAddPrinter::PageAddPrinter(QWidget *parent)
    : GenericPage(parent)
    , ui(new Ui::PageAddPrinter)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    // setup default options
    setWindowTitle(i18nc("@title:window", "Select a Printer to Add"));

    const int printerSize = 128;
    const int overlaySize = 48;

    QPixmap printerIcon = QIcon::fromTheme(QStringLiteral("printer")).pixmap(printerSize);
    const QPixmap preferencesIcon = QIcon::fromTheme(QStringLiteral("dialog-information")).pixmap(overlaySize);

    QPainter painter(&printerIcon);

    // bottom right corner
    const QPoint startPoint = QPoint(printerSize - overlaySize - 2, printerSize - overlaySize - 2);
    painter.drawPixmap(startPoint, preferencesIcon);

    ui->printerL->setPixmap(printerIcon);

    // May contain any printable characters except "/", "#", and space
    const QRegularExpression rx(QLatin1String("[^/#\\ ]*"));
    auto validator = new QRegularExpressionValidator(rx, this);
    ui->nameLE->setValidator(validator);
    connect(ui->nameLE, &QLineEdit::textChanged, this, [this](const QString &text) {
        Q_EMIT allowProceed(!text.isEmpty());
    });

    // Hide the message widget
    ui->messageWidget->setWordWrap(true);
    ui->messageWidget->setMessageType(KMessageWidget::Error);
    ui->messageWidget->hide();
}

PageAddPrinter::~PageAddPrinter()
{
    delete ui;
}

void PageAddPrinter::setValues(const QVariantMap &args)
{
    if (m_args != args) {
        QString name;
        if (!args[KCUPS_PRINTER_NAME].toString().isEmpty()) {
            name = args[KCUPS_PRINTER_NAME].toString();
        } else if (!args[KCUPS_DEVICE_MAKE_AND_MODEL].toString().isEmpty()) {
            name = args[KCUPS_DEVICE_MAKE_AND_MODEL].toString();
        } else if (!args[KCUPS_DEVICE_INFO].toString().isEmpty()) {
            name = args[KCUPS_DEVICE_INFO].toString();
        }

        if (!args[KCUPS_PRINTER_INFO].toString().isEmpty()) {
            ui->descriptionLE->setText(args[KCUPS_PRINTER_INFO].toString());
        } else {
            ui->descriptionLE->setText(name);
        }

        name.replace(QLatin1Char(' '), QLatin1Char('_'));
        name.replace(QLatin1Char('/'), QLatin1Char('-'));
        name.replace(QLatin1Char('#'), QLatin1Char('='));
        ui->nameLE->setText(name);
        ui->locationLE->setText(args[KCUPS_DEVICE_LOCATION].toString());
        ui->shareCB->setChecked(true);
        ui->shareCB->setVisible(args[ADDING_PRINTER].toBool());

        m_args = args;
    }
}

void PageAddPrinter::load()
{
}

bool PageAddPrinter::canProceed() const
{
    return !ui->nameLE->text().isEmpty();
}

bool PageAddPrinter::finishClicked()
{
    bool ret = false;
    QVariantMap args = values();
    args[KCUPS_PRINTER_IS_ACCEPTING_JOBS] = true;
    args[KCUPS_PRINTER_STATE] = IPP_PRINTER_IDLE;

    // Check if it's a printer or a class that we are adding
    bool isClass = !args.take(ADDING_PRINTER).toBool();
    QString destName = args[KCUPS_PRINTER_NAME].toString();
    QString filename = args.take(FILENAME).toString();

    QPointer<KCupsRequest> request = new KCupsRequest;
    if (isClass) {
        request->addOrModifyClass(destName, args);
    } else {
        request->addOrModifyPrinter(destName, args, filename);
    }
    request->waitTillFinished();
    if (request) {
        if (request->hasError()) {
            qCDebug(PM_ADD_PRINTER) << request->error() << request->errorMsg();
            QString message;
            if (isClass) {
                message = i18nc("@info", "Failed to add class: '%1'", request->errorMsg());
            } else {
                message = i18nc("@info", "Failed to configure printer: '%1'", request->errorMsg());
            }
            ui->messageWidget->setText(message);
            ui->messageWidget->animatedShow();
        } else {
            ret = true;
        }
        request->deleteLater();
    }

    return ret;
}

QVariantMap PageAddPrinter::values() const
{
    QVariantMap ret = m_args;
    ret[KCUPS_PRINTER_NAME] = ui->nameLE->text();
    ret[KCUPS_PRINTER_LOCATION] = ui->locationLE->text();
    ret[KCUPS_PRINTER_INFO] = ui->descriptionLE->text();
    if (ret[ADDING_PRINTER].toBool()) {
        ret[KCUPS_PRINTER_IS_SHARED] = ui->shareCB->isChecked();
    }
    return ret;
}

void PageAddPrinter::checkSelected()
{
    //     Q_EMIT allowProceed(!devicesLV->selectionModel()->selection().isEmpty());
}

#include "moc_PageAddPrinter.cpp"
