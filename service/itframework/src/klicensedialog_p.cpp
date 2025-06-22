/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007 Urs Wolfer <uwolfer at kde.org>
    SPDX-FileCopyrightText: 2008 Friedrich W. H. Kossebau <kossebau@kde.org>
    SPDX-FileCopyrightText: 2010 Teo Mrnjavac <teo@kde.org>

    Parts of this class have been take from the KAboutApplication class, which was
    SPDX-FileCopyrightText: 2000 Waldo Bastian <bastian@kde.org>
    SPDX-FileCopyrightText: 2000 Espen Sand <espen@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "klicensedialog_p.h"

// KF
#include <KAboutData>
#include <KLocalizedString>
// Qt
#include <QDialogButtonBox>
#include <QFontDatabase>
#include <QScrollBar>
#include <QStyle>
#include <QTextBrowser>
#include <QVBoxLayout>

KLicenseDialog::KLicenseDialog(const KAboutLicense &license, QWidget *parent)
    : QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    QVBoxLayout *layout = new QVBoxLayout(this);

    setWindowTitle(i18nc("@title:window", "License Agreement"));

    const QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);

    const QString licenseText = license.text();

    QTextBrowser *licenseBrowser = new QTextBrowser(this);
    licenseBrowser->setFont(font);
    licenseBrowser->setLineWrapMode(QTextEdit::NoWrap);
    licenseBrowser->setText(licenseText);
    layout->addWidget(licenseBrowser);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    buttonBox->setStandardButtons(QDialogButtonBox::Close);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttonBox);

    // try to set up the dialog such that the full width of the
    // document is visible without horizontal scroll-bars being required
    auto *style = this->style();
    const int leftMarginHint = style->pixelMetric(QStyle::PM_LayoutLeftMargin);
    const int rightMarginHint = style->pixelMetric(QStyle::PM_LayoutRightMargin);
    const qreal idealWidth = licenseBrowser->document()->idealWidth() + leftMarginHint + rightMarginHint + licenseBrowser->verticalScrollBar()->width() * 2;

    // try to allow enough height for a reasonable number of lines to be shown
    QFontMetrics metrics(font);
    const int idealHeight = metrics.height() * 30;

    resize(sizeHint().expandedTo(QSize(qRound(idealWidth), idealHeight)));
}

KLicenseDialog::~KLicenseDialog() = default;
