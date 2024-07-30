/*
 * SPDX-FileCopyrightText: 2016 Red Hat Inc
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2016 Jan Grulich <jgrulich@redhat.com>
 */

#ifndef XDG_DESKTOP_PORTAL_KDE_PRINT_H
#define XDG_DESKTOP_PORTAL_KDE_PRINT_H

#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>
#include <QDBusUnixFileDescriptor>
#include <QPrinter>

class PrintPortal : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.impl.portal.Print")
public:
    explicit PrintPortal(QObject *parent);

public Q_SLOTS:
    uint Print(const QDBusObjectPath &handle,
               const QString &app_id,
               const QString &parent_window,
               const QString &title,
               const QDBusUnixFileDescriptor &fd,
               const QVariantMap &options,
               QVariantMap &results);

    uint PreparePrint(const QDBusObjectPath &handle,
                      const QString &app_id,
                      const QString &parent_window,
                      const QString &title,
                      const QVariantMap &settings,
                      const QVariantMap &page_setup,
                      const QVariantMap &options,
                      QVariantMap &results);

private:
    QMap<uint, QPrinter *> m_printers;

    bool cupsAvailable();
    QStringList printArguments(const QPrinter *printer, bool useCupsOptions, const QString &version, QPageLayout::Orientation documentOrientation);
    QStringList destination(const QPrinter *printer, const QString &version);
    QStringList copies(const QPrinter *printer, const QString &version);
    QStringList jobname(const QPrinter *printer, const QString &version);
    QStringList cupsOptions(const QPrinter *printer, QPageLayout::Orientation documentOrientation);
    QStringList pages(const QPrinter *printer, bool useCupsOptions, const QString &version);
    QStringList optionMedia(const QPrinter *printer);
    QString mediaPaperSource(const QPrinter *printer);
    QStringList optionOrientation(const QPrinter *printer, QPageLayout::Orientation documentOrientation);
    QStringList optionDoubleSidedPrinting(const QPrinter *printer);
    QStringList optionPageOrder(const QPrinter *printer);
    QStringList optionCollateCopies(const QPrinter *printer);
    QStringList optionPageMargins(const QPrinter *printer);
    QStringList optionCupsProperties(const QPrinter *printer);
};

#endif // XDG_DESKTOP_PORTAL_KDE_PRINT_H
