/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000 Stephan Kulow <coolo@kde.org>
    SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2006, 2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kdialogjobuidelegate.h"

#include <QPointer>
#include <QQueue>
#include <QWidget>

#include <KJob>
#include <KMessageBox>
#include <kjobwidgets.h>

#include <config-kjobwidgets.h>
#if HAVE_X11
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <private/qtx11extras_p.h>
#else
#include <QX11Info>
#endif
#endif

enum DialogType { ErrorDialog, WarningDialog };

struct MessageBoxData {
    QWidget *widget;
    DialogType type = ErrorDialog;
    QString msg;
};

class KDialogJobUiDelegatePrivate : public QObject
{
    Q_OBJECT
public:
    explicit KDialogJobUiDelegatePrivate(QObject *parent = nullptr);
    ~KDialogJobUiDelegatePrivate() override;
    void queuedMessageBox(QWidget *widget, DialogType type, const QString &msg);

    QWidget *window;

public Q_SLOTS:
    void next();

private:
    bool running;
    QQueue<QSharedPointer<MessageBoxData>> queue;
};

KDialogJobUiDelegatePrivate::KDialogJobUiDelegatePrivate(QObject *parent)
    : QObject(parent)
    , window(nullptr)
    , running(false)
{
}

KDialogJobUiDelegatePrivate::~KDialogJobUiDelegatePrivate()
{
    queue.clear();
}

void KDialogJobUiDelegatePrivate::next()
{
    if (queue.isEmpty()) {
        running = false;
        return;
    }

    QSharedPointer<MessageBoxData> data = queue.dequeue();

    // kmessagebox starts a new event loop which can cause this to get deleted
    // https://bugs.kde.org/show_bug.cgi?id=356321#c16
    QPointer<KDialogJobUiDelegatePrivate> thisGuard(this);

    switch (data->type) {
    case ErrorDialog:
        KMessageBox::error(data->widget, data->msg);
        break;
    case WarningDialog:
        KMessageBox::information(data->widget, data->msg);
        break;
    };

    if (!thisGuard) {
        return;
    }

    QMetaObject::invokeMethod(this, &KDialogJobUiDelegatePrivate::next, Qt::QueuedConnection);
}

void KDialogJobUiDelegatePrivate::queuedMessageBox(QWidget *widget, DialogType type, const QString &msg)
{
    QSharedPointer<MessageBoxData> data(new MessageBoxData{widget, type, msg});

    queue.enqueue(data);

    if (!running) {
        running = true;
        QMetaObject::invokeMethod(this, &KDialogJobUiDelegatePrivate::next, Qt::QueuedConnection);
    }
}

KDialogJobUiDelegate::KDialogJobUiDelegate()
    : KJobUiDelegate()
    , d(new KDialogJobUiDelegatePrivate)
{
}

KDialogJobUiDelegate::KDialogJobUiDelegate(KJobUiDelegate::Flags flags, QWidget *window)
    : KJobUiDelegate(flags)
    , d(new KDialogJobUiDelegatePrivate)
{
    d->window = window;
}

KDialogJobUiDelegate::~KDialogJobUiDelegate() = default;

bool KDialogJobUiDelegate::setJob(KJob *job)
{
    bool ret = KJobUiDelegate::setJob(job);
#if HAVE_X11
    if (ret) {
        unsigned long time = QX11Info::appUserTime();
        KJobWidgets::updateUserTimestamp(job, time);
    }
#endif
    return ret;
}

void KDialogJobUiDelegate::setWindow(QWidget *window)
{
    if (job()) {
        KJobWidgets::setWindow(job(), window);
    }
    d->window = window;
}

QWidget *KDialogJobUiDelegate::window() const
{
    if (d->window) {
        return d->window;
    } else if (job()) {
        return KJobWidgets::window(job());
    }
    return nullptr;
}

void KDialogJobUiDelegate::updateUserTimestamp(unsigned long time)
{
    KJobWidgets::updateUserTimestamp(job(), time);
}

unsigned long KDialogJobUiDelegate::userTimestamp() const
{
    return KJobWidgets::userTimestamp(job());
}

void KDialogJobUiDelegate::showErrorMessage()
{
    if (job()->error() != KJob::KilledJobError) {
        d->queuedMessageBox(window(), ErrorDialog, job()->errorString());
    }
}

void KDialogJobUiDelegate::slotWarning(KJob * /*job*/, const QString &plain, const QString & /*rich*/)
{
    if (isAutoWarningHandlingEnabled()) {
        d->queuedMessageBox(window(), WarningDialog, plain);
    }
}

#include "kdialogjobuidelegate.moc"
