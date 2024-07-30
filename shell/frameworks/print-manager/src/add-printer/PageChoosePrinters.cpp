/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "PageChoosePrinters.h"
#include "ui_PageChoosePrinters.h"

#include <ClassListWidget.h>

#include <KCupsRequest.h>
#include <KLocalizedString>

#include <QPainter>

PageChoosePrinters::PageChoosePrinters(const QVariantMap &args, QWidget *parent)
    : GenericPage(parent)
    , ui(new Ui::PageChoosePrinters)
{
    ui->setupUi(this);

    // setup default options
    setWindowTitle(i18nc("@title:window", "Select a Printer to Add"));
    // loads the standard key icon

    const int printerSize = 128;
    const int overlaySize = 48;

    QPixmap printerIcon = QIcon::fromTheme(QStringLiteral("printer")).pixmap(printerSize);
    const QPixmap preferencesIcon = QIcon::fromTheme(QStringLiteral("preferences-other")).pixmap(overlaySize);

    QPainter painter(&printerIcon);

    // bottom right corner
    const QPoint startPoint = QPoint(printerSize - overlaySize - 2, printerSize - overlaySize - 2);
    painter.drawPixmap(startPoint, preferencesIcon);
    ui->printerL->setPixmap(printerIcon);

    connect(ui->membersLV, &ClassListWidget::changed, this, &PageChoosePrinters::allowProceed);

    if (!args.isEmpty()) {
        setValues(args);
    }
}

PageChoosePrinters::~PageChoosePrinters()
{
    delete ui;
}

void PageChoosePrinters::setValues(const QVariantMap &args)
{
    if (m_args != args) {
        m_args = args;
    }
}

QVariantMap PageChoosePrinters::values() const
{
    QVariantMap ret = m_args;
    ret[KCUPS_MEMBER_URIS] = ui->membersLV->currentSelected(true);
    return ret;
}

bool PageChoosePrinters::canProceed() const
{
    return ui->membersLV->selectedPrinters().count() > 0;
}

#include "moc_PageChoosePrinters.cpp"
