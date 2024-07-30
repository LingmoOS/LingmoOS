/*
 * SPDX-FileCopyrightText: 2018 Red Hat Inc
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2018 Jan Grulich <jgrulich@redhat.com>
 */

#ifndef XDG_DESKTOP_PORTAL_KDE_SCREENSHOT_DIALOG_H
#define XDG_DESKTOP_PORTAL_KDE_SCREENSHOT_DIALOG_H

#include "quickdialog.h"
#include <QFuture>
#include <QImage>

class ScreenshotDialog : public QuickDialog
{
    Q_OBJECT
public:
    explicit ScreenshotDialog(QObject *parent = nullptr);

    enum Flags {
        Borders = 1,
        Cursor = 1 << 1,
    };
    Q_ENUM(Flags)
    enum ScreenshotType {
        FullScreen,
        CurrentScreen,
        ActiveWindow,
    };
    Q_ENUM(ScreenshotType)

    QImage image() const;
    void takeScreenshotNonInteractive();

public Q_SLOTS:
    void takeScreenshotInteractive();

Q_SIGNALS:
    void failed();

private:
    QFuture<QImage> takeScreenshot();

    QImage m_image;
};

#endif // XDG_DESKTOP_PORTAL_KDE_SCREENSHOT_DIALOG_H
