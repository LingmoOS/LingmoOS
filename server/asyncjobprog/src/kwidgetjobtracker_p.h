/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2007 Rafael Fernández López <ereslibre@kde.org>
    SPDX-FileCopyrightText: 2007 Kevin Ottens <ervin@kde.org>
    SPDX-FileCopyrightText: 2009 Shaun Reich <shaun.reich@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KWIDGETJOBTRACKER_P_H
#define KWIDGETJOBTRACKER_P_H

#include "kabstractwidgetjobtracker_p.h"
#include "kwidgetjobtracker.h"

#include <QCheckBox>
#include <QDebug>
#include <QElapsedTimer>
#include <QEventLoopLocker>
#include <QMap>
#include <QQueue>
#include <QUrl>
#include <QWidget>

class QPushButton;
class KSqueezedTextLabel;
class QLabel;
class QProgressBar;

class KWidgetJobTrackerPrivate : public KAbstractWidgetJobTrackerPrivate
{
    Q_DECLARE_PUBLIC(KWidgetJobTracker)

public:
    KWidgetJobTrackerPrivate(QWidget *parent, KWidgetJobTracker *qq)
        : KAbstractWidgetJobTrackerPrivate(qq)
        , parent(parent)
        , eventLoopLocker(nullptr)
    {
    }

    ~KWidgetJobTrackerPrivate() override
    {
        delete eventLoopLocker;
    }

    void setStopOnClose(KJob *job, bool stopOnClose) override;
    bool stopOnClose(KJob *job) const override;
    void setAutoDelete(KJob *job, bool autoDelete) override;
    bool autoDelete(KJob *job) const override;

    void _k_showProgressWidget();

    class ProgressWidget;

    QWidget *parent;
    QEventLoopLocker *eventLoopLocker;
    QMap<KJob *, ProgressWidget *> progressWidget;
    QQueue<KJob *> progressWidgetsToBeShown;
};

class KWidgetJobTrackerPrivate::ProgressWidget : public QWidget
{
    Q_OBJECT

public:
    ProgressWidget(KJob *job, KWidgetJobTracker *object, QWidget *parent)
        : QWidget(parent)
        , tracker(object)
        , job(job)
        , totalSize(0)
        , totalFiles(0)
        , totalDirs(0)
        , totalItems(0)
        , processedSize(0)
        , processedDirs(0)
        , processedFiles(0)
        , processedItems(0)
        , totalSizeKnown(false)
        , stopOnClose(true)
        , jobRegistered(false)
        , cancelClose(nullptr)
        , openFile(nullptr)
        , openLocation(nullptr)
        , keepOpenCheck(nullptr)
        , pauseButton(nullptr)
        , sourceEdit(nullptr)
        , destEdit(nullptr)
        , progressLabel(nullptr)
        , destInvite(nullptr)
        , speedLabel(nullptr)
        , sizeLabel(nullptr)
        , resumeLabel(nullptr)
        , progressBar(nullptr)
        , suspendedProperty(false)
        , refCount(1)
    {
        if (!parent) {
            setWindowFlags(windowFlags() | Qt::Dialog);
        }

        init();
    }

    ~ProgressWidget() override
    {
        delete tracker->d_func()->eventLoopLocker;
        tracker->d_func()->eventLoopLocker = nullptr;
    }

    KWidgetJobTracker *const tracker;
    KJob *const job;

    qulonglong totalSize;
    qulonglong totalFiles;
    qulonglong totalDirs;
    qulonglong totalItems;
    qulonglong processedSize;
    qulonglong processedDirs;
    qulonglong processedFiles;
    qulonglong processedItems;

    bool totalSizeKnown;
    bool stopOnClose;
    bool jobRegistered;
    QString caption;

    QPushButton *cancelClose;
    QPushButton *openFile;
    QPushButton *openLocation;
    QCheckBox *keepOpenCheck;
    QUrl location;
    QElapsedTimer startTime;
    QPushButton *pauseButton;
    KSqueezedTextLabel *sourceEdit;
    KSqueezedTextLabel *destEdit;
    QLabel *progressLabel;
    QLabel *sourceInvite;
    QLabel *destInvite;
    QLabel *speedLabel;
    QLabel *sizeLabel;
    QLabel *resumeLabel;
    QProgressBar *progressBar;
    QPushButton *arrowButton;
    Qt::ArrowType arrowState;

    bool suspendedProperty;

    int refCount; // will not close the dialog if a modal menu is shown

    void init();
    void showTotals();
    void setDestVisible(bool visible);
    void checkDestination(const QUrl &dest);
    void ref();
    void deref();
    void closeNow();

    bool eventFilter(QObject *watched, QEvent *event) override;

public Q_SLOTS:
    virtual void infoMessage(const QString &plain, const QString &rich);
    virtual void description(const QString &title, const QPair<QString, QString> &field1, const QPair<QString, QString> &field2);
    virtual void totalAmount(KJob::Unit unit, qulonglong amount);
    virtual void processedAmount(KJob::Unit unit, qulonglong amount);
    virtual void percent(unsigned long percent);
    virtual void speed(unsigned long value);
    virtual void slotClean();
    virtual void suspended();
    virtual void resumed();

    // TODO: Misses canResume()

protected:
    void closeEvent(QCloseEvent *event) override;

private Q_SLOTS:
    void keepOpenToggled(bool);
    void openFileClicked();
    void openLocationClicked();
    void pauseResumeClicked();
    void cancelClicked();
    void arrowClicked();
};

void KWidgetJobTrackerPrivate::setStopOnClose(KJob *job, bool stopOnClose)
{
    if (!progressWidget.contains(job)) {
        return;
    }
    progressWidget[job]->stopOnClose = stopOnClose;
}

bool KWidgetJobTrackerPrivate::stopOnClose(KJob *job) const
{
    if (!progressWidget.contains(job)) {
        qWarning() << "no widget found for job" << job;
        return true;
    }
    return progressWidget[job]->stopOnClose;
}

void KWidgetJobTrackerPrivate::setAutoDelete(KJob *job, bool autoDelete)
{
    if (!progressWidget.contains(job)) {
        return;
    }
    progressWidget[job]->setAttribute(Qt::WA_DeleteOnClose, autoDelete);
}

bool KWidgetJobTrackerPrivate::autoDelete(KJob *job) const
{
    if (!progressWidget.contains(job)) {
        qWarning() << "no widget found for job" << job;
        return true;
    }
    return progressWidget[job]->testAttribute(Qt::WA_DeleteOnClose);
}

#endif // KWIDGETJOBTRACKER_P_H
