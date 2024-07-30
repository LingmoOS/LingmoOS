/*
 * SPDX-FileCopyrightText: 2016-2017 Jan Grulich <jgrulich@redhat.com>
 * SPDX-FileCopyrightText: 2007, 2010 John Layt <john@layt.net>
 * SPDX-FileCopyrightText: 2007 Alex Merry <huntedhacker@tiscali.co.uk>
 * SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 */

#include "print.h"
#include "print_debug.h"
#include "request.h"
#include "utils.h"

#include <KProcess>

#include <QFile>
#include <QPrintDialog>
#include <QPrintEngine>
#include <QStandardPaths>
// #include <QtPrintSupport/private/qprint_p.h>
#include <QTemporaryFile>
#include <QUrl>
#include <QtPrintSupport/private/qcups_p.h>

// INFO: code below is copied from Qt as there is no public API for converting key to PageSizeId

// Standard sizes data
struct StandardPageSize {
    QPageSize::PageSizeId id;
    const char *mediaOption; // PPD standard mediaOption ID
};

// Standard page sizes taken from the Postscript PPD Standard v4.3
// See https://www-cdf.fnal.gov/offline/PostScript/5003.PPD_Spec_v4.3.pdf
// Excludes all Transverse and Rotated sizes
// NB!This table needs to be in sync with QPageSize::PageSizeId
const static StandardPageSize qt_pageSizes[] = {

    // Existing Qt sizes including ISO, US, ANSI and other standards
    {QPageSize::A4, "A4"},
    {QPageSize::B5, "ISOB5"},
    {QPageSize::Letter, "Letter"},
    {QPageSize::Legal, "Legal"},
    {QPageSize::Executive, "Executive.7.5x10in"}, // Qt size differs from Postscript / Windows
    {QPageSize::A0, "A0"},
    {QPageSize::A1, "A1"},
    {QPageSize::A2, "A2"},
    {QPageSize::A3, "A3"},
    {QPageSize::A5, "A5"},
    {QPageSize::A6, "A6"},
    {QPageSize::A7, "A7"},
    {QPageSize::A8, "A8"},
    {QPageSize::A9, "A9"},
    {QPageSize::B0, "ISOB0"},
    {QPageSize::B1, "ISOB1"},
    {QPageSize::B10, "ISOB10"},
    {QPageSize::B2, "ISOB2"},
    {QPageSize::B3, "ISOB3"},
    {QPageSize::B4, "ISOB4"},
    {QPageSize::B6, "ISOB6"},
    {QPageSize::B7, "ISOB7"},
    {QPageSize::B8, "ISOB8"},
    {QPageSize::B9, "ISOB9"},
    {QPageSize::C5E, "EnvC5"},
    {QPageSize::Comm10E, "Env10"},
    {QPageSize::DLE, "EnvDL"},
    {QPageSize::Folio, "Folio"},
    {QPageSize::Ledger, "Ledger"},
    {QPageSize::Tabloid, "Tabloid"},
    {QPageSize::Custom, "Custom"}, // Special case to keep in sync with QPageSize::PageSizeId

    // ISO Standard Sizes
    {QPageSize::A10, "A10"},
    {QPageSize::A3Extra, "A3Extra"},
    {QPageSize::A4Extra, "A4Extra"},
    {QPageSize::A4Plus, "A4Plus"},
    {QPageSize::A4Small, "A4Small"},
    {QPageSize::A5Extra, "A5Extra"},
    {QPageSize::B5Extra, "ISOB5Extra"},

    // JIS Standard Sizes
    {QPageSize::JisB0, "B0"},
    {QPageSize::JisB1, "B1"},
    {QPageSize::JisB2, "B2"},
    {QPageSize::JisB3, "B3"},
    {QPageSize::JisB4, "B4"},
    {QPageSize::JisB5, "B5"},
    {QPageSize::JisB6, "B6"},
    {QPageSize::JisB7, "B7"},
    {QPageSize::JisB8, "B8"},
    {QPageSize::JisB9, "B9"},
    {QPageSize::JisB10, "B10"},

    // ANSI / US Standard sizes
    {QPageSize::AnsiC, "AnsiC"},
    {QPageSize::AnsiD, "AnsiD"},
    {QPageSize::AnsiE, "AnsiE"},
    {QPageSize::LegalExtra, "LegalExtra"},
    {QPageSize::LetterExtra, "LetterExtra"},
    {QPageSize::LetterPlus, "LetterPlus"},
    {QPageSize::LetterSmall, "LetterSmall"},
    {QPageSize::TabloidExtra, "TabloidExtra"},

    // Architectural sizes
    {QPageSize::ArchA, "ARCHA"},
    {QPageSize::ArchB, "ARCHB"},
    {QPageSize::ArchC, "ARCHC"},
    {QPageSize::ArchD, "ARCHD"},
    {QPageSize::ArchE, "ARCHE"},

    // Inch-based Sizes
    {QPageSize::Imperial7x9, "7x9"},
    {QPageSize::Imperial8x10, "8x10"},
    {QPageSize::Imperial9x11, "9x11"},
    {QPageSize::Imperial9x12, "9x12"},
    {QPageSize::Imperial10x11, "10x11"},
    {QPageSize::Imperial10x13, "10x13"},
    {QPageSize::Imperial10x14, "10x14"},
    {QPageSize::Imperial12x11, "12x11"},
    {QPageSize::Imperial15x11, "15x11"},

    // Other Page Sizes
    {QPageSize::ExecutiveStandard, "Executive"}, // Qt size differs from Postscript / Windows
    {QPageSize::Note, "Note"},
    {QPageSize::Quarto, "Quarto"},
    {QPageSize::Statement, "Statement"},
    {QPageSize::SuperA, "SuperA"},
    {QPageSize::SuperB, "SuperB"},
    {QPageSize::Postcard, "Postcard"},
    {QPageSize::DoublePostcard, "DoublePostcard"},
    {QPageSize::Prc16K, "PRC16K"},
    {QPageSize::Prc32K, "PRC32K"},
    {QPageSize::Prc32KBig, "PRC32KBig"},

    // Fan Fold Sizes
    {QPageSize::FanFoldUS, "FanFoldUS"},
    {QPageSize::FanFoldGerman, "FanFoldGerman"},
    {QPageSize::FanFoldGermanLegal, "FanFoldGermanLegal"},

    // ISO Envelopes
    {QPageSize::EnvelopeB4, "EnvISOB4"},
    {QPageSize::EnvelopeB5, "EnvISOB5"},
    {QPageSize::EnvelopeB6, "EnvISOB6"},
    {QPageSize::EnvelopeC0, "EnvC0"},
    {QPageSize::EnvelopeC1, "EnvC1"},
    {QPageSize::EnvelopeC2, "EnvC2"},
    {QPageSize::EnvelopeC3, "EnvC3"},
    {QPageSize::EnvelopeC4, "EnvC4"},
    {QPageSize::EnvelopeC6, "EnvC6"},
    {QPageSize::EnvelopeC65, "EnvC65"},
    {QPageSize::EnvelopeC7, "EnvC7"},

    // US Envelopes
    {QPageSize::Envelope9, "Env9"},
    {QPageSize::Envelope11, "Env11"},
    {QPageSize::Envelope12, "Env12"},
    {QPageSize::Envelope14, "Env14"},
    {QPageSize::EnvelopeMonarch, "EnvMonarch"},
    {QPageSize::EnvelopePersonal, "EnvPersonal"},

    // Other Envelopes
    {QPageSize::EnvelopeChou3, "EnvChou3"},
    {QPageSize::EnvelopeChou4, "EnvChou4"},
    {QPageSize::EnvelopeInvite, "EnvInvite"},
    {QPageSize::EnvelopeItalian, "EnvItalian"},
    {QPageSize::EnvelopeKaku2, "EnvKaku2"},
    {QPageSize::EnvelopeKaku3, "EnvKaku3"},
    {QPageSize::EnvelopePrc1, "EnvPRC1"},
    {QPageSize::EnvelopePrc2, "EnvPRC2"},
    {QPageSize::EnvelopePrc3, "EnvPRC3"},
    {QPageSize::EnvelopePrc4, "EnvPRC4"},
    {QPageSize::EnvelopePrc5, "EnvPRC5"},
    {QPageSize::EnvelopePrc6, "EnvPRC6"},
    {QPageSize::EnvelopePrc7, "EnvPRC7"},
    {QPageSize::EnvelopePrc8, "EnvPRC8"},
    {QPageSize::EnvelopePrc9, "EnvPRC9"},
    {QPageSize::EnvelopePrc10, "EnvPRC10"},
    {QPageSize::EnvelopeYou4, "EnvYou4"}};

// Return key name for PageSize
static QString qt_keyForPageSizeId(QPageSize::PageSizeId id)
{
    return QString::fromLatin1(qt_pageSizes[id].mediaOption);
}

// Return id name for PPD Key
static QPageSize::PageSizeId qt_idForPpdKey(const QString &ppdKey)
{
    if (ppdKey.isEmpty()) {
        return QPageSize::Custom;
    }

    for (int i = 0; i <= int(QPageSize::LastPageSize); ++i) {
        if (QLatin1String(qt_pageSizes[i].mediaOption) == ppdKey) {
            return qt_pageSizes[i].id;
        }
    }
    return QPageSize::Custom;
}

PrintPortal::PrintPortal(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
}

uint PrintPortal::Print(const QDBusObjectPath &handle,
                        const QString &app_id,
                        const QString &parent_window,
                        const QString &title,
                        const QDBusUnixFileDescriptor &fd,
                        const QVariantMap &options,
                        QVariantMap &results)
{
    Q_UNUSED(results)
    qCDebug(XdgDesktopPortalKdePrint) << "Print called with parameters:";
    qCDebug(XdgDesktopPortalKdePrint) << "    handle: " << handle.path();
    qCDebug(XdgDesktopPortalKdePrint) << "    app_id: " << app_id;
    qCDebug(XdgDesktopPortalKdePrint) << "    parent_window: " << parent_window;
    qCDebug(XdgDesktopPortalKdePrint) << "    title: " << title;
    qCDebug(XdgDesktopPortalKdePrint) << "    fd: " << fd.fileDescriptor();
    qCDebug(XdgDesktopPortalKdePrint) << "    options: " << options;

    QFile fileToPrint;
    if (fileToPrint.open(fd.fileDescriptor(), QIODevice::ReadOnly)) {
        QPrinter *printer = nullptr;
        // Use printer associated with token if possible
        if (options.contains(QStringLiteral("token"))) {
            printer = m_printers.value(options.value(QStringLiteral("token")).toUInt());
        } else {
            // Use the last configured printer otherwise
            if (m_printers.count()) {
                printer = m_printers.last();
            }
        }

        if (!printer) {
            qCDebug(XdgDesktopPortalKdePrint) << "Failed to print: no QPrinter what can be used for printing";
            return 1;
        }

        // We are going to print to a file
        if (!printer->outputFileName().isEmpty()) {
            if (QFile::exists(printer->outputFileName())) {
                QFile::remove(printer->outputFileName());
            }

            QByteArray pdfContent = fileToPrint.readAll();
            QFile outputFile(printer->outputFileName());
            if (outputFile.open(QIODevice::ReadWrite)) {
                outputFile.write(pdfContent);
                outputFile.close();
            } else {
                qCDebug(XdgDesktopPortalKdePrint) << "Failed to print: couldn't open the file for writing";
            }

            fileToPrint.close();

            return 0;

            // TODO poscript support?

            // Print to a printer via lpr command
        } else {
            // The code below is copied from Okular
            bool useCupsOptions = cupsAvailable();
            QString exe;
            QStringList argList;

            // Decide what executable to use to print with, need the CUPS version of lpr if available
            // Some distros name the CUPS version of lpr as lpr-cups or lpr.cups so try those first
            // before default to lpr, or failing that to lp
            if (!QStandardPaths::findExecutable(QStringLiteral("lpr-cups")).isEmpty()) {
                exe = QStringLiteral("lpr-cups");
            } else if (!QStandardPaths::findExecutable(QStringLiteral("lpr.cups")).isEmpty()) {
                exe = QStringLiteral("lpr.cups");
            } else if (!QStandardPaths::findExecutable(QStringLiteral("lpr")).isEmpty()) {
                exe = QStringLiteral("lpr");
            } else if (!QStandardPaths::findExecutable(QStringLiteral("lp")).isEmpty()) {
                exe = QStringLiteral("lp");
            } else {
                qCDebug(XdgDesktopPortalKdePrint) << "Failed to print: couldn't run lpr command for printing";
                return 1;
            }

            QTemporaryFile tempFile;
            if (tempFile.open()) {
                tempFile.write(fileToPrint.readAll());
                tempFile.close();
            } else {
                qCDebug(XdgDesktopPortalKdePrint) << "Failed to print: couldn't create temporary file for printing";
                return 1;
            }

            argList = printArguments(printer, useCupsOptions, exe, printer->pageLayout().orientation()) << tempFile.fileName();
            // qCDebug(XdgDesktopPortalKdePrint) << "Executing" << exe << "with arguments" << argList << tempFile.fileName();
            int retValue = KProcess::execute(exe, argList);

            if (retValue <= 0) {
                qCDebug(XdgDesktopPortalKdePrint) << "Failed to print: running KProcess failed";
                return 1;
            }

            return retValue;
        }
    } else {
        qCDebug(XdgDesktopPortalKdePrint) << "Failed to print: couldn't not read from fd";
        return 1;
    }

    return 0;
}

uint PrintPortal::PreparePrint(const QDBusObjectPath &handle,
                               const QString &app_id,
                               const QString &parent_window,
                               const QString &title,
                               const QVariantMap &settings,
                               const QVariantMap &page_setup,
                               const QVariantMap &options,
                               QVariantMap &results)
{
    qCDebug(XdgDesktopPortalKdePrint) << "PreparePrint called with parameters:";
    qCDebug(XdgDesktopPortalKdePrint) << "    handle: " << handle.path();
    qCDebug(XdgDesktopPortalKdePrint) << "    app_id: " << app_id;
    qCDebug(XdgDesktopPortalKdePrint) << "    parent_window: " << parent_window;
    qCDebug(XdgDesktopPortalKdePrint) << "    title: " << title;
    qCDebug(XdgDesktopPortalKdePrint) << "    settings: " << settings;
    qCDebug(XdgDesktopPortalKdePrint) << "    page_setup: " << page_setup;
    qCDebug(XdgDesktopPortalKdePrint) << "    options: " << options;

    // Create new one
    QPrinter *printer = new QPrinter();

    // First we have to load pre-configured options

    // Process settings (used by printer)
    QCUPSSupport::PagesPerSheet pagesPerSheet = QCUPSSupport::OnePagePerSheet;
    QCUPSSupport::PagesPerSheetLayout pagesPerSheetLayout = QCUPSSupport::LeftToRightTopToBottom;
    for (auto it = settings.constBegin(); it != settings.constEnd(); ++it) {
        if (it.key() == QStringLiteral("orientation")) {
            const QString orientation = it.value().toString();
            if (orientation == QLatin1String("landscape") || orientation == QLatin1String("reverse_landscape")) {
                printer->setPageOrientation(QPageLayout::Landscape);
            } else if (orientation == QLatin1String("portrait") || orientation == QLatin1String("reverse_portrait")) {
                printer->setPageOrientation(QPageLayout::Portrait);
            }
        } else if (it.key() == QStringLiteral("n-copies")) {
            printer->setCopyCount(it.value().toString().toInt());
        } else if (it.key() == QStringLiteral("resolution")) {
            printer->setResolution(it.value().toString().toInt());
        } else if (it.key() == QStringLiteral("use-color")) {
            printer->setColorMode(it.value().toString() == QLatin1String("yes") ? QPrinter::Color : QPrinter::GrayScale);
        } else if (it.key() == QStringLiteral("duplex")) {
            const QString duplex = it.value().toString();
            if (duplex == QLatin1String("simplex")) {
                printer->setDuplex(QPrinter::DuplexNone);
            } else if (duplex == QLatin1String("horizontal")) {
                printer->setDuplex(QPrinter::DuplexShortSide);
            } else if (duplex == QLatin1String("vertical")) {
                printer->setDuplex(QPrinter::DuplexLongSide);
            }
        } else if (it.key() == QStringLiteral("collate")) {
            printer->setCollateCopies(it.value().toString() == QLatin1String("yes"));
        } else if (it.key() == QStringLiteral("reverse")) {
            printer->setPageOrder(it.value().toString() == QLatin1String("yes") ? QPrinter::LastPageFirst : QPrinter::FirstPageFirst);
        } else if (it.key() == QStringLiteral("print-pages")) {
            const QString printPages = it.value().toString();
            if (printPages == QLatin1String("all")) {
                printer->setPrintRange(QPrinter::AllPages);
            } else if (printPages == QLatin1String("selection")) {
                printer->setPrintRange(QPrinter::Selection);
            } else if (printPages == QLatin1String("current")) {
                printer->setPrintRange(QPrinter::CurrentPage);
            } else if (printPages == QLatin1String("ranges")) {
                printer->setPrintRange(QPrinter::PageRange);
            }
        } else if (it.key() == QStringLiteral("page-ranges")) {
            const QString range = it.value().toString();
            // Gnome supports format like 1-5,7,9,11-15, however Qt support only e.g.1-15
            // so get the first and the last value
            const QStringList ranges = range.split(QLatin1Char(','));
            if (ranges.count()) {
                QStringList firstRangeValues = ranges.first().split(QLatin1Char('-'));
                QStringList lastRangeValues = ranges.last().split(QLatin1Char('-'));
                printer->setFromTo(firstRangeValues.first().toInt(), lastRangeValues.last().toInt());
            }
        } else if (it.key() == QStringLiteral("page-set")) {
            // WARNING Qt internal private API, anyway the print dialog doesn't seem to
            // read these properties, but I'll leave it here in case this changes in future
            const QString pageSet = it.value().toString();
            if (pageSet == QLatin1String("all")) {
                QCUPSSupport::setPageSet(printer, QCUPSSupport::AllPages);
            } else if (pageSet == QLatin1String("even")) {
                QCUPSSupport::setPageSet(printer, QCUPSSupport::EvenPages);
            } else if (pageSet == QLatin1String("odd")) {
                QCUPSSupport::setPageSet(printer, QCUPSSupport::OddPages);
            }
        } else if (it.key() == QStringLiteral("number-up")) {
            // WARNING Qt internal private API, anyway the print dialog doesn't seem to
            // read these properties, but I'll leave it here in case this changes in future
            const QString numberUp = it.value().toString();
            if (numberUp == QLatin1String("1")) {
                pagesPerSheet = QCUPSSupport::OnePagePerSheet;
            } else if (numberUp == QLatin1String("2")) {
                pagesPerSheet = QCUPSSupport::TwoPagesPerSheet;
            } else if (numberUp == QLatin1String("4")) {
                pagesPerSheet = QCUPSSupport::FourPagesPerSheet;
            } else if (numberUp == QLatin1String("6")) {
                pagesPerSheet = QCUPSSupport::SixPagesPerSheet;
            } else if (numberUp == QLatin1String("9")) {
                pagesPerSheet = QCUPSSupport::NinePagesPerSheet;
            } else if (numberUp == QLatin1String("16")) {
                pagesPerSheet = QCUPSSupport::SixteenPagesPerSheet;
            }
        } else if (it.key() == QStringLiteral("number-up-layout")) {
            // WARNING Qt internal private API, anyway the print dialog doesn't seem to
            // read these properties, but I'll leave it here in case this changes in future
            const QString layout = it.value().toString();
            if (layout == QLatin1String("lrtb")) {
                pagesPerSheetLayout = QCUPSSupport::LeftToRightTopToBottom;
            } else if (layout == QLatin1String("lrbt")) {
                pagesPerSheetLayout = QCUPSSupport::LeftToRightBottomToTop;
            } else if (layout == QLatin1String("rltb")) {
                pagesPerSheetLayout = QCUPSSupport::RightToLeftTopToBottom;
            } else if (layout == QLatin1String("rlbt")) {
                pagesPerSheetLayout = QCUPSSupport::RightToLeftBottomToTop;
            } else if (layout == QLatin1String("tblr")) {
                pagesPerSheetLayout = QCUPSSupport::TopToBottomLeftToRight;
            } else if (layout == QLatin1String("tbrl")) {
                pagesPerSheetLayout = QCUPSSupport::TopToBottomRightToLeft;
            } else if (layout == QLatin1String("btlr")) {
                pagesPerSheetLayout = QCUPSSupport::BottomToTopLeftToRight;
            } else if (layout == QLatin1String("btrl")) {
                pagesPerSheetLayout = QCUPSSupport::BottomToTopRightToLeft;
            }
        } else if (it.key() == QStringLiteral("output-file-format")) {
            // TODO only PDF supported by Qt so when printing to file we can use PDF only
            // btw. this should be already set automatically because we set output file name
            printer->setOutputFormat(QPrinter::PdfFormat);
        } else if (it.key() == QStringLiteral("output-uri")) {
            const QUrl uri = QUrl(it.value().toString());
            // Check whether the uri is not just a directory name and whether we don't need to
            // append output-basename
            if (settings.contains(QStringLiteral("output-basename"))) {
                const QString basename = settings.value(QStringLiteral("output-basename")).toString();
                if (!uri.toDisplayString().endsWith(basename) && uri.toDisplayString().endsWith(QLatin1Char('/'))) {
                    printer->setOutputFileName(uri.toDisplayString() + basename);
                }
            } else {
                printer->setOutputFileName(uri.toDisplayString());
            }
        } else {
            qCDebug(XdgDesktopPortalKdePrint) << "Unhandled printer setting: " << it.key();
        }
    }

    QCUPSSupport::setPagesPerSheetLayout(printer, pagesPerSheet, pagesPerSheetLayout);

    // Process page setup
    QSizeF paperSize;
    QString ppdName;
    QString name;
    QMarginsF pageMargins = printer->pageLayout().margins(QPageLayout::Millimeter);
    for (auto it = page_setup.constBegin(); it != page_setup.constEnd(); ++it) {
        if (it.key() == QStringLiteral("PPDName")) {
            ppdName = it.value().toString();
        } else if (it.key() == QStringLiteral("Name")) {
            name = it.value().toString();
        }

        if (it.key() == QStringLiteral("Width")) {
            paperSize.setWidth(it.value().toReal());
        } else if (it.key() == QStringLiteral("Height")) {
            paperSize.setHeight(it.value().toReal());
        } else if (it.key() == QStringLiteral("MarginTop")) {
            pageMargins.setTop(it.value().toReal());
        } else if (it.key() == QStringLiteral("MarginBottom")) {
            pageMargins.setBottom(it.value().toReal());
        } else if (it.key() == QStringLiteral("MarginLeft")) {
            pageMargins.setLeft(it.value().toReal());
        } else if (it.key() == QStringLiteral("MarginRight")) {
            pageMargins.setRight(it.value().toReal());
        } else if (it.key() == QStringLiteral("Orientation")) {
            const QString orientation = it.value().toString();
            if (orientation == QLatin1String("landscape") || orientation == QLatin1String("reverse_landscape")) {
                printer->setPageOrientation(QPageLayout::Landscape);
            } else if (orientation == QLatin1String("portrait") || orientation == QLatin1String("reverse_portrait")) {
                printer->setPageOrientation(QPageLayout::Portrait);
            }
        } else {
            qCDebug(XdgDesktopPortalKdePrint) << "Unhandled page setup: " << it.key();
        }
    }

    if (!ppdName.isEmpty()) {
        printer->setPageSize(QPageSize(qt_idForPpdKey(ppdName)));
    } else if (!name.isEmpty()) {
        printer->setPageSize(QPageSize(paperSize, QPageSize::Millimeter, name));
    }
    printer->setPageMargins(pageMargins, QPageLayout::Millimeter);

    QPrintDialog *printDialog = new QPrintDialog(printer);
    Utils::setParentWindow(printDialog, parent_window);
    Request::makeClosableDialogRequest(handle, printDialog);

    // Process options
    if (options.contains(QStringLiteral("modal"))) {
        printDialog->setModal(options.value(QStringLiteral("modal")).toBool());
    }

    // Pass back what we configured
    if (printDialog->exec() == QDialog::Accepted) {
        QVariantMap resultingSettings;
        QVariantMap resultingPageSetup;

        // Process back printer settings
        resultingSettings.insert(QStringLiteral("n-copies"), QString::number(printer->copyCount()));
        resultingSettings.insert(QStringLiteral("resolution"), QString::number(printer->resolution()));
        resultingSettings.insert(QStringLiteral("use-color"), printer->colorMode() == QPrinter::Color ? QLatin1String("yes") : QLatin1String("no"));
        if (printer->duplex() == QPrinter::DuplexNone) {
            resultingSettings.insert(QStringLiteral("duplex"), QLatin1String("simplex"));
        } else if (printer->duplex() == QPrinter::DuplexShortSide) {
            resultingSettings.insert(QStringLiteral("duplex"), QLatin1String("horizontal"));
        } else if (printer->duplex() == QPrinter::DuplexLongSide) {
            resultingSettings.insert(QStringLiteral("duplex"), QLatin1String("vertical"));
        }
        resultingSettings.insert(QStringLiteral("collate"), printer->collateCopies() ? QLatin1String("yes") : QLatin1String("no"));
        resultingSettings.insert(QStringLiteral("reverse"), printer->pageOrder() == QPrinter::LastPageFirst ? QLatin1String("yes") : QLatin1String("no"));
        if (printer->printRange() == QPrinter::AllPages) {
            resultingSettings.insert(QStringLiteral("print-pages"), QLatin1String("all"));
        } else if (printer->printRange() == QPrinter::Selection) {
            resultingSettings.insert(QStringLiteral("print-pages"), QLatin1String("selection"));
        } else if (printer->printRange() == QPrinter::CurrentPage) {
            resultingSettings.insert(QStringLiteral("print-pages"), QLatin1String("current"));
        } else if (printer->printRange() == QPrinter::PageRange) {
            resultingSettings.insert(QStringLiteral("print-pages"), QLatin1String("ranges"));
            const int fromPageToIndex = printer->fromPage() ? printer->fromPage() - 1 : printer->fromPage();
            const int toPageToIndex = printer->toPage() ? printer->toPage() - 1 : printer->toPage();
            resultingSettings.insert(QStringLiteral("page-ranges"), QStringLiteral("%1-%2").arg(fromPageToIndex).arg(toPageToIndex));
        }
        // Set cups specific properties
        const QStringList cupsOptions = printer->printEngine()->property(PPK_CupsOptions).toStringList();
        qCDebug(XdgDesktopPortalKdePrint) << cupsOptions;
        if (cupsOptions.contains(QLatin1String("page-set"))) {
            resultingSettings.insert(QStringLiteral("page-set"), cupsOptions.at(cupsOptions.indexOf(QStringLiteral("page-set")) + 1));
        }
        if (cupsOptions.contains(QLatin1String("number-up"))) {
            resultingSettings.insert(QStringLiteral("number-up"), cupsOptions.at(cupsOptions.indexOf(QStringLiteral("number-up")) + 1));
        }
        if (cupsOptions.contains(QLatin1String("number-up-layout"))) {
            resultingSettings.insert(QStringLiteral("number-up-layout"), cupsOptions.at(cupsOptions.indexOf(QStringLiteral("number-up-layout")) + 1));
        }

        if (printer->outputFormat() == QPrinter::PdfFormat) {
            resultingSettings.insert(QStringLiteral("output-file-format"), QLatin1String("pdf"));
        }

        if (!printer->outputFileName().isEmpty()) {
            resultingSettings.insert(QStringLiteral("output-uri"), QUrl::fromLocalFile(printer->outputFileName()).toDisplayString());
        }

        // Process back page setup
        if (printer->pageLayout().pageSize().id() == QPageSize::Custom) {
            resultingPageSetup.insert(QStringLiteral("Name"), printer->pageLayout().pageSize().name());
            resultingPageSetup.insert(QStringLiteral("DisplayName"), printer->pageLayout().pageSize().name());
        } else {
            resultingPageSetup.insert(QStringLiteral("PPDName"), qt_keyForPageSizeId(printer->pageLayout().pageSize().id()));
            resultingPageSetup.insert(QStringLiteral("DisplayName"), qt_keyForPageSizeId(printer->pageLayout().pageSize().id()));
        }
        resultingPageSetup.insert(QStringLiteral("Width"), printer->pageLayout().pageSize().size(QPageSize::Millimeter).width());
        resultingPageSetup.insert(QStringLiteral("Height"), printer->pageLayout().pageSize().size(QPageSize::Millimeter).height());
        resultingPageSetup.insert(QStringLiteral("MarginTop"), printer->pageLayout().margins(QPageLayout::Millimeter).top());
        resultingPageSetup.insert(QStringLiteral("MarginBottom"), printer->pageLayout().margins(QPageLayout::Millimeter).bottom());
        resultingPageSetup.insert(QStringLiteral("MarginLeft"), printer->pageLayout().margins(QPageLayout::Millimeter).left());
        resultingPageSetup.insert(QStringLiteral("MarginRight"), printer->pageLayout().margins(QPageLayout::Millimeter).right());
        resultingPageSetup.insert(QStringLiteral("Orientation"),
                                  printer->pageLayout().orientation() == QPageLayout::Landscape ? QLatin1String("landscape") : QLatin1String("portrait"));

        qCDebug(XdgDesktopPortalKdePrint) << "Settings: ";
        qCDebug(XdgDesktopPortalKdePrint) << "---------------------------";
        qCDebug(XdgDesktopPortalKdePrint) << resultingSettings;
        qCDebug(XdgDesktopPortalKdePrint) << "Page setup: ";
        qCDebug(XdgDesktopPortalKdePrint) << "---------------------------";
        qCDebug(XdgDesktopPortalKdePrint) << resultingPageSetup;

        uint token = QDateTime::currentDateTime().toSecsSinceEpoch();
        results.insert(QStringLiteral("settings"), resultingSettings);
        results.insert(QStringLiteral("page-setup"), resultingPageSetup);
        results.insert(QStringLiteral("token"), token);

        m_printers.insert(token, printer);

        printDialog->deleteLater();
        return 0;
    } else {
        printDialog->deleteLater();
        return 1;
    }

    return 0;
}

QStringList PrintPortal::destination(const QPrinter *printer, const QString &version)
{
    if (version == QLatin1String("lp")) {
        return QStringList(QStringLiteral("-d")) << printer->printerName();
    }

    if (version.startsWith(QLatin1String("lpr"))) {
        return QStringList(QStringLiteral("-P")) << printer->printerName();
    }

    return QStringList();
}

QStringList PrintPortal::copies(const QPrinter *printer, const QString &version)
{
    int cp = printer->copyCount();

    if (version == QLatin1String("lp")) {
        return QStringList(QStringLiteral("-n")) << QStringLiteral("%1").arg(cp);
    }

    if (version.startsWith(QLatin1String("lpr"))) {
        return QStringList() << QStringLiteral("-#%1").arg(cp);
    }

    return QStringList();
}

QStringList PrintPortal::jobname(const QPrinter *printer, const QString &version)
{
    if (!printer->docName().isEmpty()) {
        if (version == QLatin1String("lp")) {
            return QStringList(QStringLiteral("-t")) << printer->docName();
        }

        if (version.startsWith(QLatin1String("lpr"))) {
            const QString shortenedDocName = QString::fromUtf8(printer->docName().toUtf8().left(255));
            return QStringList(QStringLiteral("-J")) << shortenedDocName;
        }
    }

    return QStringList();
}

// What about Upper and MultiPurpose?  And others in PPD???
QString PrintPortal::mediaPaperSource(const QPrinter *printer)
{
    switch (printer->paperSource()) {
    case QPrinter::Auto:
        return QString();
    case QPrinter::Cassette:
        return QStringLiteral("Cassette");
    case QPrinter::Envelope:
        return QStringLiteral("Envelope");
    case QPrinter::EnvelopeManual:
        return QStringLiteral("EnvelopeManual");
    case QPrinter::FormSource:
        return QStringLiteral("FormSource");
    case QPrinter::LargeCapacity:
        return QStringLiteral("LargeCapacity");
    case QPrinter::LargeFormat:
        return QStringLiteral("LargeFormat");
    case QPrinter::Lower:
        return QStringLiteral("Lower");
    case QPrinter::MaxPageSource:
        return QStringLiteral("MaxPageSource");
    case QPrinter::Middle:
        return QStringLiteral("Middle");
    case QPrinter::Manual:
        return QStringLiteral("Manual");
    case QPrinter::OnlyOne:
        return QStringLiteral("OnlyOne");
    case QPrinter::Tractor:
        return QStringLiteral("Tractor");
    case QPrinter::SmallFormat:
        return QStringLiteral("SmallFormat");
    default:
        return QString();
    }
}

QStringList PrintPortal::optionOrientation(const QPrinter *printer, QPageLayout::Orientation documentOrientation)
{
    // portrait and landscape options rotate the document according to the document orientation
    // If we want to print a landscape document as one would expect it, we have to pass the
    // portrait option so that the document is not rotated additionally
    if (printer->pageLayout().orientation() == documentOrientation) {
        // the user wants the document printed as is
        return QStringList(QStringLiteral("-o")) << QStringLiteral("portrait");
    } else {
        // the user expects the document being rotated by 90 degrees
        return QStringList(QStringLiteral("-o")) << QStringLiteral("landscape");
    }
}

QStringList PrintPortal::optionDoubleSidedPrinting(const QPrinter *printer)
{
    switch (printer->duplex()) {
    case QPrinter::DuplexNone:
        return QStringList(QStringLiteral("-o")) << QStringLiteral("sides=one-sided");
    case QPrinter::DuplexAuto:
        if (printer->pageLayout().orientation() == QPageLayout::Landscape) {
            return QStringList(QStringLiteral("-o")) << QStringLiteral("sides=two-sided-short-edge");
        } else {
            return QStringList(QStringLiteral("-o")) << QStringLiteral("sides=two-sided-long-edge");
        }
    case QPrinter::DuplexLongSide:
        return QStringList(QStringLiteral("-o")) << QStringLiteral("sides=two-sided-long-edge");
    case QPrinter::DuplexShortSide:
        return QStringList(QStringLiteral("-o")) << QStringLiteral("sides=two-sided-short-edge");
    default:
        return QStringList(); // Use printer default
    }
}

QStringList PrintPortal::optionPageOrder(const QPrinter *printer)
{
    if (printer->pageOrder() == QPrinter::LastPageFirst) {
        return QStringList(QStringLiteral("-o")) << QStringLiteral("outputorder=reverse");
    }
    return QStringList(QStringLiteral("-o")) << QStringLiteral("outputorder=normal");
}

QStringList PrintPortal::optionCollateCopies(const QPrinter *printer)
{
    if (printer->collateCopies()) {
        return QStringList(QStringLiteral("-o")) << QStringLiteral("Collate=True");
    }
    return QStringList(QStringLiteral("-o")) << QStringLiteral("Collate=False");
}

QStringList PrintPortal::optionPageMargins(const QPrinter *printer)
{
    if (printer->printEngine()->property(QPrintEngine::PPK_PageMargins).isNull()) {
        return QStringList();
    } else {
        qreal l, t, r, b;
        QMarginsF m = printer->pageLayout().margins();
        l = m.left();
        t = m.top();
        r = m.right();
        b = m.bottom();
        return QStringList{
            QStringLiteral("-o"),
            QStringLiteral("page-left=%1").arg(l),
            QStringLiteral("-o"),
            QStringLiteral("page-top=%1").arg(t),
            QStringLiteral("-o"),
            QStringLiteral("page-right=%1").arg(r),
            QStringLiteral("-o"),
            QStringLiteral("page-bottom=%1").arg(b),
            QStringLiteral("-o"),
            QStringLiteral("fit-to-page"),
        };
    }
}

QStringList PrintPortal::optionCupsProperties(const QPrinter *printer)
{
    QStringList dialogOptions = printer->printEngine()->property(QPrintEngine::PrintEnginePropertyKey(0xfe00)).toStringList();
    QStringList cupsOptions;

    for (int i = 0; i < dialogOptions.count(); i = i + 2) {
        // Ignore some cups properties as the pdf we get is already formatted using these
        if (dialogOptions[i] == QLatin1String("number-up") || dialogOptions[i] == QLatin1String("number-up-layout")) {
            continue;
        }

        if (dialogOptions[i + 1].isEmpty()) {
            cupsOptions << QStringLiteral("-o") << dialogOptions[i];
        } else {
            cupsOptions << QStringLiteral("-o") << dialogOptions[i] + QLatin1Char('=') + dialogOptions[i + 1];
        }
    }

    return cupsOptions;
}

QStringList PrintPortal::optionMedia(const QPrinter *printer)
{
    if (!qt_keyForPageSizeId(printer->pageLayout().pageSize().id()).isEmpty() && !mediaPaperSource(printer).isEmpty()) {
        return QStringList(QStringLiteral("-o"))
            << QStringLiteral("media=%1,%2").arg(qt_keyForPageSizeId(printer->pageLayout().pageSize().id()), mediaPaperSource(printer));
    }

    if (!qt_keyForPageSizeId(printer->pageLayout().pageSize().id()).isEmpty()) {
        return QStringList(QStringLiteral("-o")) << QStringLiteral("media=%1").arg(qt_keyForPageSizeId(printer->pageLayout().pageSize().id()));
    }

    if (!mediaPaperSource(printer).isEmpty()) {
        return QStringList(QStringLiteral("-o")) << QStringLiteral("media=%1").arg(mediaPaperSource(printer));
    }

    return QStringList();
}

QStringList PrintPortal::pages(const QPrinter *printer, bool useCupsOptions, const QString &version)
{
    if (printer->printRange() == QPrinter::PageRange) {
        if (version == QLatin1String("lp")) {
            return QStringList(QStringLiteral("-P")) << QStringLiteral("%1-%2").arg(printer->fromPage()).arg(printer->toPage());
        }

        if (version.startsWith(QLatin1String("lpr")) && useCupsOptions) {
            return QStringList(QStringLiteral("-o")) << QStringLiteral("page-ranges=%1-%2").arg(printer->fromPage()).arg(printer->toPage());
        }
    }

    return QStringList(); // AllPages
}

QStringList PrintPortal::cupsOptions(const QPrinter *printer, QPageLayout::Orientation documentOrientation)
{
    Q_UNUSED(documentOrientation)
    QStringList optionList;

    // if (!optionMedia(printer).isEmpty()) {
    //     optionList << optionMedia(printer);
    // }

    // if (!optionOrientation(printer, documentOrientation).isEmpty()) {
    //     optionList << optionOrientation(printer, documentOrientation);
    // }

    if (!optionDoubleSidedPrinting(printer).isEmpty()) {
        optionList << optionDoubleSidedPrinting(printer);
    }

    if (!optionPageOrder(printer).isEmpty()) {
        optionList << optionPageOrder(printer);
    }

    if (!optionCollateCopies(printer).isEmpty()) {
        optionList << optionCollateCopies(printer);
    }

    // if (!optionPageMargins(printer).isEmpty()) {
    //     optionList << optionPageMargins(printer);
    // }

    optionList << optionCupsProperties(printer);

    return optionList;
}

QStringList PrintPortal::printArguments(const QPrinter *printer, bool useCupsOptions, const QString &version, QPageLayout::Orientation documentOrientation)
{
    QStringList argList;

    if (!destination(printer, version).isEmpty()) {
        argList << destination(printer, version);
    }

    if (!copies(printer, version).isEmpty()) {
        argList << copies(printer, version);
    }

    if (!jobname(printer, version).isEmpty()) {
        argList << jobname(printer, version);
    }

    // if (!pages(printer, useCupsOptions, version).isEmpty()) {
    //     argList << pages(printer, useCupsOptions, version);
    // }

    if (useCupsOptions && !cupsOptions(printer, documentOrientation).isEmpty()) {
        argList << cupsOptions(printer, documentOrientation);
    }

    if (version == QLatin1String("lp")) {
        argList << QStringLiteral("--");
    }

    return argList;
}

bool PrintPortal::cupsAvailable()
{
    // Ideally we would have access to the private Qt method
    // QCUPSSupport::cupsAvailable() to do this as it is very complex routine.
    // However, if CUPS is available then QPrinter::numCopies() will always return 1
    // whereas if CUPS is not available it will return the real number of copies.
    // This behaviour is guaranteed never to change, so we can use it as a reliable substitute.
    QPrinter testPrinter;
    return testPrinter.supportsMultipleCopies();
}
