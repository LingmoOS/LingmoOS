/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2004 George Staikos <staikos@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KWALLETWIZARD_H
#define KWALLETWIZARD_H

#include <QWizard>
#ifdef HAVE_GPGMEPP
#include <gpgme++/key.h>
#endif

class PageGpgKey;
class PagePassword;
class PageIntro;

class KWalletWizard : public QWizard
{
    Q_OBJECT
public:
    enum WizardType { Basic, Advanced };

    static const int PageIntroId = 0;
    static const int PagePasswordId = 1;
#ifdef HAVE_GPGMEPP
    static const int PageGpgKeyId = 2;
#endif
    static const int PageOptionsId = 3;
    static const int PageExplanationId = 4;

    explicit KWalletWizard(QWidget *parent = nullptr);

    WizardType wizardType() const;

#ifdef HAVE_GPGMEPP
    GpgME::Key gpgKey() const;
#endif // HAVE_GPGMEPP

protected:
    void initializePage(int id) override;

protected Q_SLOTS:
    friend class PagePassword;
    void passwordPageUpdate();

private:
    PageIntro *m_pageIntro;
    PagePassword *m_pagePasswd;
#ifdef HAVE_GPGMEPP
    PageGpgKey *m_pageGpgKey;
#endif
};

#endif
