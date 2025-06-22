/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2019 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KABSTRACTABOUTDIALOG_P_H
#define KABSTRACTABOUTDIALOG_P_H

#include <KAboutData>

class QDialog;
class QWidget;
class QIcon;

/**
 * @internal
 *
 * Private base class implementing util methods for assembling an About dialog.
 */
class KAbstractAboutDialogPrivate
{
public:
    KAbstractAboutDialogPrivate() = default;
    ~KAbstractAboutDialogPrivate() = default;

public:
    QWidget *createTitleWidget(const QIcon &icon, const QString &displayName, const QString &version, QWidget *parent);
    QWidget *createAboutWidget(const QString &shortDescription,
                               const QString &otherText,
                               const QString &copyrightStatement,
                               const QString &homepage,
                               const QList<KAboutLicense> &licenses,
                               QWidget *parent);
    QWidget *createComponentWidget(const QList<KAboutComponent> &components, QWidget *parent);
    QWidget *createAuthorsWidget(const QList<KAboutPerson> &authors,
                                 bool customAuthorTextEnabled,
                                 const QString &customAuthorRichText,
                                 const QString &bugAddress,
                                 QWidget *parent);
    QWidget *createCreditWidget(const QList<KAboutPerson> &credits, QWidget *parent);
    QWidget *createTranslatorsWidget(const QList<KAboutPerson> &translators, QWidget *parent);
    void createForm(QWidget *titleWidget, QWidget *tabWidget, QDialog *dialog);
};

#endif
