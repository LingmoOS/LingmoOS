/*
    SPDX-FileCopyrightText: 2002-2010 Anders Lund <anders@alweb.dk>

    Rewritten based on code of:
    SPDX-FileCopyrightText: 2002 Michael Goffioul <kdeprint@swing.be>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kateprinter.h"

#include "kateconfig.h"
#include "katedocument.h"
#include "kateglobal.h"
#include "kateview.h"

#include <KConfigGroup>
#include <KSharedConfig>

#include <QApplication>
#include <QMarginsF>
#include <QPageLayout>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrinter>

#include "printconfigwidgets.h"
#include "printpainter.h"

using namespace KatePrinter;

// BEGIN KatePrinterPrivate
class KatePrinterPrivate : public QObject
{
public:
    KatePrinterPrivate(KTextEditor::DocumentPrivate *doc, KTextEditor::ViewPrivate *view = nullptr);

    bool print(QPrinter *printer);
    void setColorScheme(const QString &scheme);

public:
    void paint(QPrinter *printer);

private:
    KTextEditor::ViewPrivate *m_view;
    KTextEditor::DocumentPrivate *m_doc;
    PrintPainter m_painter;
    static void readSettings(QPrinter *printer);
    static void writeSettings(QPrinter *printer);
};

KatePrinterPrivate::KatePrinterPrivate(KTextEditor::DocumentPrivate *doc, KTextEditor::ViewPrivate *view)
    : QObject()
    , m_view(view)
    , m_doc(doc)
    , m_painter(m_doc, m_view)
{
}

bool KatePrinterPrivate::print(QPrinter *printer)
{
    // docname is now always there, including the right Untitled name
    printer->setDocName(m_doc->documentName());

    KatePrintTextSettings *kpts = new KatePrintTextSettings;
    KatePrintHeaderFooter *kphf = new KatePrintHeaderFooter;
    KatePrintLayout *kpl = new KatePrintLayout;

    QList<QWidget *> tabs;
    tabs << kpts;
    tabs << kphf;
    tabs << kpl;

    QWidget *parentWidget = m_doc->widget();

    if (!parentWidget) {
        parentWidget = QApplication::activeWindow();
    }

    readSettings(printer);

    QPointer<QPrintDialog> printDialog(new QPrintDialog(printer, parentWidget));
    printDialog->setOptionTabs(tabs);

    if (m_view && m_view->selection()) {
        printer->setPrintRange(QPrinter::Selection);
        printDialog->setOption(QAbstractPrintDialog::PrintSelection, true);
    }

    printDialog->setOption(QAbstractPrintDialog::PrintPageRange, true);

    const int dlgCode = printDialog->exec();
    if (dlgCode != QDialog::Accepted || !printDialog) {
        delete printDialog;
        return false;
    }

    writeSettings(printer);

    // configure the painter
    m_painter.setPrintGuide(kpts->printGuide());
    m_painter.setPrintLineNumbers(kpts->printLineNumbers());
    m_painter.setDontPrintFoldedCode(kpts->dontPrintFoldedCode());

    m_painter.setColorScheme(kpl->colorScheme());
    m_painter.setTextFont(kpl->textFont());
    m_painter.setUseBackground(kpl->useBackground());
    m_painter.setUseBox(kpl->useBox());
    m_painter.setBoxMargin(kpl->boxMargin());
    m_painter.setBoxWidth(kpl->boxWidth());
    m_painter.setBoxColor(kpl->boxColor());

    m_painter.setHeadersFont(kphf->font());

    m_painter.setUseHeader(kphf->useHeader());
    m_painter.setHeaderBackground(kphf->headerBackground());
    m_painter.setHeaderForeground(kphf->headerForeground());
    m_painter.setUseHeaderBackground(kphf->useHeaderBackground());
    m_painter.setHeaderFormat(kphf->headerFormat());

    m_painter.setUseFooter(kphf->useFooter());
    m_painter.setFooterBackground(kphf->footerBackground());
    m_painter.setFooterForeground(kphf->footerForeground());
    m_painter.setUseFooterBackground(kphf->useFooterBackground());
    m_painter.setFooterFormat(kphf->footerFormat());

    delete printDialog;
    m_painter.paint(printer);

    return true;
}

void KatePrinterPrivate::paint(QPrinter *printer)
{
    m_painter.paint(printer);
}

void KatePrinterPrivate::setColorScheme(const QString &scheme)
{
    m_painter.setColorScheme(scheme);
}

void KatePrinterPrivate::writeSettings(QPrinter *printer)
{
    KSharedConfigPtr config = KTextEditor::EditorPrivate::config();
    KConfigGroup group(config, QStringLiteral("Kate Print Settings"));
    KConfigGroup margins(&group, QStringLiteral("Margins"));

    QMarginsF m = printer->pageLayout().margins(QPageLayout::Millimeter);
    margins.writeEntry("left", m.left());
    margins.writeEntry("top", m.top());
    margins.writeEntry("right", m.right());
    margins.writeEntry("bottom", m.bottom());
}

void KatePrinterPrivate::readSettings(QPrinter *printer)
{
    KSharedConfigPtr config = KTextEditor::EditorPrivate::config();
    KConfigGroup group(config, QStringLiteral("Kate Print Settings"));
    KConfigGroup margins(&group, QStringLiteral("Margins"));

    qreal left{};
    qreal right{};
    qreal top{};
    qreal bottom{};
    left = margins.readEntry("left", left);
    top = margins.readEntry("top", top);
    right = margins.readEntry("right", right);
    bottom = margins.readEntry("bottom", bottom);
    QMarginsF m = QMarginsF(left, top, right, bottom);
    printer->setPageMargins(m, QPageLayout::Millimeter);
}

// END KatePrinterPrivate

// BEGIN KatePrinter

bool KatePrinter::print(KTextEditor::ViewPrivate *view)
{
    QPrinter printer;
    KatePrinterPrivate p(view->doc(), view);
    return p.print(&printer);
}

bool KatePrinter::printPreview(KTextEditor::ViewPrivate *view)
{
    QPrinter printer;
    KatePrinterPrivate p(view->doc(), view);
    p.setColorScheme(QStringLiteral("Printing"));
    QPrintPreviewDialog preview(&printer, view);
    QObject::connect(&preview, &QPrintPreviewDialog::paintRequested, &p, &KatePrinterPrivate::paint);
    return preview.exec();
}

bool KatePrinter::print(KTextEditor::DocumentPrivate *doc)
{
    QPrinter printer;
    KatePrinterPrivate p(doc);
    return p.print(&printer);
}

bool KatePrinter::printPreview(KTextEditor::DocumentPrivate *doc)
{
    QPrinter printer;
    KatePrinterPrivate p(doc);
    p.setColorScheme(QStringLiteral("Printing"));
    QPrintPreviewDialog preview(&printer);
    QObject::connect(&preview, &QPrintPreviewDialog::paintRequested, &p, &KatePrinterPrivate::paint);
    return preview.exec();
}

// END KatePrinter
