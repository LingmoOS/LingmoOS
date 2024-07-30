/*
    SPDX-FileCopyrightText: 2010 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ChooseSerial.h"
#include "ui_ChooseSerial.h"

#include <KCupsRequest.h>
#include <KLocalizedString>

#include <QPainter>

ChooseSerial::ChooseSerial(QWidget *parent)
    : GenericPage(parent)
    , ui(new Ui::ChooseSerial)
    , m_rx(QLatin1String("\\?baud=(\\d+)"))
{
    ui->setupUi(this);

    // setup default options
    setWindowTitle(i18nc("@title:window", "Select a Printer to Add"));

    ui->parityCB->addItem(i18nc("@label:listbox", "None"), QLatin1String("none"));
    ui->parityCB->addItem(i18nc("@label:listbox", "Even"), QLatin1String("even"));
    ui->parityCB->addItem(i18nc("@label:listbox", "Odd"), QLatin1String("odd"));

    ui->flowCB->addItem(i18nc("@label:listbox", "None"), QLatin1String("none"));
    ui->flowCB->addItem(i18nc("@label:listbox", "XON/XOFF (Software)"), QLatin1String("soft"));
    ui->flowCB->addItem(i18nc("@label:listbox", "RTS/CTS (Hardware)"), QLatin1String("hard"));
    ui->flowCB->addItem(i18nc("@label:listbox", "DTR/DSR (Hardware)"), QLatin1String("dtrdsr"));
}

ChooseSerial::~ChooseSerial()
{
    delete ui;
}

bool ChooseSerial::isValid() const
{
    return m_isValid;
}

void ChooseSerial::setValues(const QVariantMap &args)
{
    m_args = args;
    const QString deviceUri = args[KCUPS_DEVICE_URI].toString();
    if (!deviceUri.startsWith(QLatin1String("serial:"))) {
        m_isValid = false;
        return;
    }
    m_isValid = true;

    static int baudrates[] = /* Baud rates */
        {
            1200,
            2400,
            4800,
            9600,
            19200,
            38400,
            57600,
            115200,
            230400,
            460800,
        };

    // Find out the max baud rate
    int maxrate;
    const auto match = m_rx.match(deviceUri);
    if (match.hasMatch()) {
        maxrate = match.captured(1).toInt();
    } else {
        maxrate = 19200;
    }

    ui->baudRateCB->clear();
    for (int i = 0; i < 10; i++) {
        if (baudrates[i] > maxrate) {
            break;
        } else {
            ui->baudRateCB->addItem(QString::number(baudrates[i]));
        }
    }
    // Set the current index to the maxrate
    ui->baudRateCB->setCurrentIndex(ui->baudRateCB->count() - 1);
}

void ChooseSerial::load()
{
}

QVariantMap ChooseSerial::values() const
{
    QVariantMap ret = m_args;
    QString deviceUri = m_args[KCUPS_DEVICE_URI].toString();
    const int pos = deviceUri.indexOf(QLatin1Char('?'));
    const QString baudRate = ui->baudRateCB->currentText();
    const QString bits = ui->bitsCB->currentText();
    const QString parity = ui->baudRateCB->itemData(ui->baudRateCB->currentIndex()).toString();
    const QString flow = ui->flowCB->itemData(ui->flowCB->currentIndex()).toString();
    const QString replace = QString::fromLatin1("?baud=%1+bits=%2+parity=%3+flow=%4").arg(baudRate, bits, parity, flow);
    deviceUri.replace(pos, deviceUri.size() - pos, replace);
    ret[KCUPS_DEVICE_URI] = deviceUri;
    return ret;
}

#include "moc_ChooseSerial.cpp"
