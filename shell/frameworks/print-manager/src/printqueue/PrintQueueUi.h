/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PRINT_QUEUE_UI_H
#define PRINT_QUEUE_UI_H

#include <QDialog>
#include <QModelIndex>

namespace Ui
{
class PrintQueueUi;
}

class KCupsRequest;
class KCupsPrinter;
class JobSortFilterModel;
class JobModel;
class QToolButton;

class PrintQueueUi : public QDialog
{
    Q_OBJECT

public:
    explicit PrintQueueUi(const KCupsPrinter &printer, QWidget *parent = nullptr);
    ~PrintQueueUi() override;

signals:
    void finished();

public slots:
    void update();

private slots:
    void updatePrinterByName(const QString &printer);
    void updatePrinter(const QString &text,
                       const QString &printerUri,
                       const QString &printerName,
                       uint printerState,
                       const QString &printerStateReasons,
                       bool printerIsAcceptingJobs);
    void whichJobsIndexChanged(int index);
    void pausePrinter();
    void configurePrinter();

    void cancelJob();
    void holdJob();
    void resumeJob();
    void reprintJob();
    void authenticateJob();

    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    void updateButtons();
    void showContextMenu(const QPoint &point);
    void showHeaderContextMenu(const QPoint &point);
    void getAttributesFinished(KCupsRequest *request);

private:
    void closeEvent(QCloseEvent *event) override;
    void setupButtons();
    void setState(int state, const QString &message);
    void modifyJob(int action, const QString &destName = QString());

    Ui::PrintQueueUi *const ui;
    QToolButton *m_filterJobs = nullptr;
    JobSortFilterModel *m_proxyModel = nullptr;
    JobModel *m_model = nullptr;
    QString m_destName;
    QString m_title;
    QPixmap m_printerIcon;
    char m_lastState = 0;
    bool m_isClass;
    bool m_preparingMenu = false;
    bool m_printerPaused = false;
};

#endif
