// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "printertestpagedialog.h"
#include "ztroubleshoot.h"
#include "zjobmanager.h"

#include <QLabel>
#include <QTimer>
#include <QAbstractButton>

PrinterTestPageDialog::PrinterTestPageDialog(const QString &printerName, QWidget *parent)
    : QObject(nullptr)
    , m_printerName(printerName)
    , m_testJob(nullptr)
    , m_trobleShoot(nullptr)
    , m_parent(parent)
{
    connect(this, &PrinterTestPageDialog::signalFinished, this, &QObject::deleteLater);
}

void PrinterTestPageDialog::printTestPage()
{
    m_testJob = new PrinterTestJob(m_printerName, this, false);
    if (m_testJob->findRunningJob()) {
        emit signalFinished();
        return;
    }

    m_trobleShoot = new TroubleShoot(m_printerName, this);
    connect(m_trobleShoot, &TroubleShoot::signalUpdateProgress, this, &PrinterTestPageDialog::slotTroubleShootMessage);
    connect(m_trobleShoot, &TroubleShoot::signalStatus, this, &PrinterTestPageDialog::slotTroubleShootStatus);
    m_trobleShoot->start();
}

void PrinterTestPageDialog::showErrorMessage(const QString &message)
{
    DDialog dlg(m_parent);

    dlg.setIcon(QIcon(":/images/dde-printer.svg"));

    dlg.setMessage(message);
    dlg.addButton(tr("OK"), true);
    dlg.getButton(0)->setFixedWidth(200);
    dlg.setFixedHeight(202);
    dlg.exec();
}

void PrinterTestPageDialog::slotTroubleShootMessage(int proccess, QString messge)
{
    Q_UNUSED(proccess);

    m_message = messge;
}

void PrinterTestPageDialog::slotTroubleShootStatus(int id, int state)
{
    Q_UNUSED(id);

    if (TStat_Suc == state) {
        if (!m_testJob->isPass())
            showErrorMessage(m_testJob->getMessage());
    } else if (TStat_Fail == state) {
        showErrorMessage(m_message);
    }

    if (TStat_Suc == state || TStat_Fail == state)
        emit signalFinished();
}
