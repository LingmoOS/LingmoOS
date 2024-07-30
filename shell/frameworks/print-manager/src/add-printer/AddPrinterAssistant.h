/*
    SPDX-FileCopyrightText: 2010 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ADD_PRINTER_ASSISTANT_H
#define ADD_PRINTER_ASSISTANT_H

#include <KAssistantDialog>

#include <KPixmapSequenceOverlayPainter>

class AddPrinterAssistant : public KAssistantDialog
{
    Q_OBJECT
public:
    AddPrinterAssistant();
    ~AddPrinterAssistant() override;

    void initAddPrinter(const QString &printer = QString(), const QString &deviceId = QString());
    void initAddClass();
    void initChangePPD(const QString &printer, const QString &deviceUri, const QString &makeAndModel);

public slots:
    void back() override;
    void next() override;
    void enableNextButton(bool enable);
    void enableFinishButton(bool enable);
    void slotFinishButtonClicked();

private:
    void next(KPageWidgetItem *currentPage);
    void setCurrentPage(KPageWidgetItem *page);
    void showEvent(QShowEvent *event) override;

    KPageWidgetItem *m_devicesPage = nullptr;
    KPageWidgetItem *m_chooseClassPage = nullptr;
    KPageWidgetItem *m_choosePPDPage = nullptr;
    KPageWidgetItem *m_addPrinterPage = nullptr;
    KPixmapSequenceOverlayPainter *m_busySeq;
};

#endif
