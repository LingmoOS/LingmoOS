// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <DDialog>

DWIDGET_USE_NAMESPACE

class PrinterTestJob;
class TroubleShoot;

QT_BEGIN_NAMESPACE
class QLabel;
QT_END_NAMESPACE

class PrinterTestPageDialog : public QObject
{
    Q_OBJECT

public:
    PrinterTestPageDialog(const QString &printerName, QWidget *parent = nullptr);

    void printTestPage();

protected slots:
    void slotTroubleShootMessage(int proccess, QString messge);
    void slotTroubleShootStatus(int id, int state);

protected:
    void showErrorMessage(const QString &message);

signals:
    void signalFinished();

private:
    QString m_printerName;

    PrinterTestJob *m_testJob;
    TroubleShoot *m_trobleShoot;
    QString m_message;
    QWidget *m_parent;
};
