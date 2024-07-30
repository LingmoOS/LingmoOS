/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ClassListWidget.h"

#include "SelectMakeModel.h"

#include "KCupsRequest.h"
#include "NoSelectionRectDelegate.h"

#include <QFileDialog>
#include <QPointer>

#include <KConfigDialogManager>
#include <KIconLoader>
#include <KPixmapSequence>

ClassListWidget::ClassListWidget(QWidget *parent)
    : QListView(parent)
{
    m_model = new QStandardItemModel(this);
    setModel(m_model);
    setItemDelegate(new NoSelectionRectDelegate(this));

    // Setup the busy cursor
    m_busySeq = new KPixmapSequenceOverlayPainter(this);
    m_busySeq->setSequence(KPixmapSequence(QLatin1String("process-working"), KIconLoader::SizeSmallMedium));
    m_busySeq->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_busySeq->setWidget(viewport());

    connect(m_model, &QStandardItemModel::dataChanged, this, &ClassListWidget::modelChanged);

    m_delayedInit.setInterval(0);
    m_delayedInit.setSingleShot(true);
    connect(&m_delayedInit, &QTimer::timeout, this, &ClassListWidget::init);
    m_delayedInit.start();
}

ClassListWidget::~ClassListWidget()
{
}

void ClassListWidget::init()
{
    m_busySeq->start(); // Start spinning
    m_model->clear();

    QStringList att;
    att << KCUPS_PRINTER_NAME;
    att << KCUPS_PRINTER_URI_SUPPORTED;
    // Get destinations with these masks
    m_request = new KCupsRequest;
    connect(m_request, &KCupsRequest::finished, this, &ClassListWidget::loadFinished);
    if (m_showClasses) {
        m_request->getPrinters(att);
    } else {
        m_request->getPrinters(att, CUPS_PRINTER_CLASS | CUPS_PRINTER_REMOTE | CUPS_PRINTER_IMPLICIT);
    }
}

void ClassListWidget::loadFinished(KCupsRequest *request)
{
    // If we have an old request running discard it's result and get a new one
    if (m_request != request) {
        request->deleteLater();
        return;
    }

    m_busySeq->stop(); // Stop spinning

    const KCupsPrinters printers = request->printers();
    request->deleteLater();
    m_request = nullptr;

    for (const KCupsPrinter &printer : printers) {
        QString destName = printer.name();
        if (destName != m_printerName) {
            auto item = new QStandardItem;
            item->setText(destName);
            item->setCheckable(true);
            item->setEditable(false);
            item->setData(printer.uriSupported());
            updateItemState(item);

            m_model->appendRow(item);
        }
    }

    modelChanged();
}

void ClassListWidget::modelChanged()
{
    QStringList currentMembers = currentSelected(false);

    m_changed = m_selectedPrinters != currentMembers;

    Q_EMIT changed(m_changed);
}

QStringList ClassListWidget::currentSelected(bool uri) const
{
    QStringList currentMembers;
    for (int i = 0; i < m_model->rowCount(); i++) {
        QStandardItem *item = m_model->item(i);
        if (item && item->checkState() == Qt::Checked) {
            if (uri) {
                currentMembers << item->data().toString();
            } else {
                currentMembers << item->text();
            }
        }
    }
    currentMembers.sort();
    return currentMembers;
}

void ClassListWidget::updateItemState(QStandardItem *item) const
{
    if (m_selectedPrinters.contains(item->text())) {
        item->setCheckState(Qt::Checked);
    } else {
        item->setCheckState(Qt::Unchecked);
    }
}

bool ClassListWidget::hasChanges()
{
    return m_changed;
}

void ClassListWidget::setPrinter(const QString &printer)
{
    if (m_printerName != printer) {
        m_printerName = printer;
        m_delayedInit.start();
    }
}

QString ClassListWidget::selectedPrinters() const
{
    return currentSelected(false).join(QLatin1String("|"));
}

void ClassListWidget::setSelectedPrinters(const QString &selected)
{
    m_selectedPrinters = selected.split(QLatin1Char('|'));
    m_selectedPrinters.sort();
    m_delayedInit.start();
}

bool ClassListWidget::showClasses() const
{
    return m_showClasses;
}

void ClassListWidget::setShowClasses(bool enable)
{
    if (m_showClasses != enable) {
        m_showClasses = enable;
        m_delayedInit.start();
    }
}

#include "moc_ClassListWidget.cpp"
