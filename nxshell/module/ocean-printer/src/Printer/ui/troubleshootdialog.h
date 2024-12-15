// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "printerhelpwindow.h"

#include <DDialog>
#include <QFrame>
#include <QLabel>

DWIDGET_USE_NAMESPACE

class TroubleShootJob;
class TroubleShoot;

QT_BEGIN_NAMESPACE
class QLabel;
class QPushButton;
QT_END_NAMESPACE

class TroubleShootItem : public QFrame
{
    Q_OBJECT

public:
    TroubleShootItem(TroubleShootJob *job, int index, QWidget *parent = nullptr);

protected:
    void slotStateChanged(int state, const QString &message);

    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

private:
    TroubleShootJob *m_job;
    QLabel *m_iconLabel;
    QLabel *m_titleLabel;
    QLabel *m_messageLabel;
    int m_index;
};

class TroubleShootDialog : public DAbstractDialog
{
    Q_OBJECT

public:
    TroubleShootDialog(const QString &printerName, QWidget *parent = nullptr);

protected slots:
    void slotTroubleShootStatus(int id, int state);

private:
    QString m_printerName;

    TroubleShoot *m_trobleShoot;
    QPushButton *m_button;
    QPushButton *m_helpButton;
    PrinterHelpWindow *m_pHelpWindow = nullptr;
};

