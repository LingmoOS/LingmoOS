/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef PLASMAVAULT_KDED_UI_CRYFS_CYPHER_CHOOSER_WIDGET_H
#define PLASMAVAULT_KDED_UI_CRYFS_CYPHER_CHOOSER_WIDGET_H

#include "dialogdsl.h"

class CryfsCypherChooserWidget : public DialogDsl::DialogModule
{
    Q_OBJECT

public:
    CryfsCypherChooserWidget();
    ~CryfsCypherChooserWidget() override;

    PlasmaVault::Vault::Payload fields() const override;

private Q_SLOTS:
    void initializeCyphers();

private:
    class Private;
    QScopedPointer<Private> d;
};

inline DialogDsl::ModuleFactory cryfsCypherChooser()
{
    return [=] {
        return new CryfsCypherChooserWidget();
    };
}

#endif // include guard
