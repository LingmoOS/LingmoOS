/*
 * SPDX-FileCopyrightText: 2018 Red Hat Inc
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2018 Jan Grulich <jgrulich@redhat.com>
 */

#ifndef XDG_DESKTOP_PORTAL_KDE_SCREENCHOOSER_DIALOG_H
#define XDG_DESKTOP_PORTAL_KDE_SCREENCHOOSER_DIALOG_H

#include "outputsmodel.h"
#include "quickdialog.h"
#include "screencast.h"
#include <QEventLoop>
#include <QRect>

class ScreenChooserDialog : public QuickDialog
{
    Q_OBJECT
public:
    ScreenChooserDialog(const QString &appName, bool multiple, ScreenCastPortal::SourceTypes types);
    ~ScreenChooserDialog() override;

    QList<Output> selectedOutputs() const;
    QList<QMap<int, QVariant>> selectedWindows() const;
    bool allowRestore() const;
    QRect selectedRegion() const;

public Q_SLOTS:
    void accept() override;

Q_SIGNALS:
    void clearSelection();

private:
    void setRegion(const QRect region);

    QRect m_region;
};

#endif // XDG_DESKTOP_PORTAL_KDE_SCREENCHOOSER_DIALOG_H
