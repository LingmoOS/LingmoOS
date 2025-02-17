/*
    SPDX-FileCopyrightText: 2021 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KJOBCREATOR_H
#define KJOBCREATOR_H

#include <QDialog>
#include <QPointer>
#include <QScopedPointer>

#include <KJob>

#include "ui_kjobcreator.h"

class KWidgetJobTracker;
class KUiServerJobTracker;
class KUiServerV2JobTracker;

class TestJob : public KJob
{
    Q_OBJECT

public:
    TestJob(QObject *parent = nullptr);

    void start() override;
    bool started() const;

    void finish();

    void setPercentage(int percent);

    void setProcessedMiB(double bytes);
    void setTotalMiB(double bytes);
    void setProcessedFiles(int files);
    void setTotalFiles(int files);
    void setProcessedDirectories(int directories);
    void setTotalDirectories(int directories);
    void setProcessedItems(int items);
    void setTotalItems(int items);

    void setSpeedMiB(double speed);

    void setErrorState(int errorCode, const QString &errorText);

    void setKillable(bool killable);
    void setSuspendable(bool suspendable);

protected:
    bool doKill() override;
    bool doSuspend() override;
    bool doResume() override;

private:
    bool m_started = false;
};

class TestDialog : public QDialog
{
    Q_OBJECT

public:
    TestDialog(QWidget *parent = nullptr);
    ~TestDialog() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void updateUiState();
    void updateJob();

    Ui_KJobCreator m_ui;

    QPointer<TestJob> m_job;

    QScopedPointer<KWidgetJobTracker> m_widgetTracker;
    bool m_registeredWithWidgetTracker = false;
    QScopedPointer<KUiServerJobTracker> m_uiServerTracker;
    bool m_registeredWithUiServerTracker = false;
    QScopedPointer<KUiServerV2JobTracker> m_uiServerV2Tracker;
    bool m_registeredWithUiServerV2Tracker = false;
};

#endif // KJOBCREATOR_H
