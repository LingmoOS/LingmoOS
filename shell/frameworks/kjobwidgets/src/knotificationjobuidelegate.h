/*
    This file is part of the KDE Frameworks
    SPDX-FileCopyrightText: 2020 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KNOTIFICATIONJOBUIDELEGATE_H
#define KNOTIFICATIONJOBUIDELEGATE_H

#include <KJobUiDelegate>

#include <QScopedPointer>

#include <kjobwidgets_export.h>

class KNotificationJobUiDelegatePrivate;

/**
 * @class KNotificationJobUiDelegate knotificationjobuidelegate.h KNotificationJobUiDelegate
 *
 * A UI delegate using KNotification for interaction (showing errors and warnings).
 *
 * @since 6.0
 */
class KJOBWIDGETS_EXPORT KNotificationJobUiDelegate : public KJobUiDelegate
{
    Q_OBJECT

public:
    /**
     * Constructs a new KNotificationJobUiDelegate.
     * @param flags allows to enable automatic error/warning handling
     * @since 6.0
     */
    explicit KNotificationJobUiDelegate(KJobUiDelegate::Flags flags = {KJobUiDelegate::AutoHandlingDisabled});

    /**
     * Destroys the KNotificationJobUiDelegate.
     */
    ~KNotificationJobUiDelegate() override;

public:
    /**
     * Display a notification to inform the user of the error given by
     * this job.
     */
    void showErrorMessage() override;

protected Q_SLOTS:
    bool setJob(KJob *job) override;
    void slotWarning(KJob *job, const QString &message) override;

private:
    QScopedPointer<KNotificationJobUiDelegatePrivate> d;
};

#endif // KNOTIFICATIONJOBUIDELEGATE_H
