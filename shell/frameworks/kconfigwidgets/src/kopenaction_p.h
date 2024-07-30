/*
 * SPDX-FileCopyrightText: 2023 Kai Uwe Broulik <kde@broulik.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef KOPENACTION_H
#define KOPENACTION_H

#include <KToolBarPopupAction>
#include <kconfigwidgets_export.h>

#include <memory>

class KOpenActionPrivate;

// TODO export only for for unittests?
class KCONFIGWIDGETS_EXPORT KOpenAction : public KToolBarPopupAction
{
    Q_OBJECT

public:
    explicit KOpenAction(QObject *parent = nullptr);
    KOpenAction(const QIcon &icon, const QString &text, QObject *parent);
    ~KOpenAction() override;

protected:
    QWidget *createWidget(QWidget *parent) override;

private:
    std::unique_ptr<KOpenActionPrivate> const d;
};

#endif // KOPENACTION_H
