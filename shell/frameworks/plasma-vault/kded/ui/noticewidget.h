/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef PLASMAVAULT_KDED_UI_NOTICE_WIDGET_H
#define PLASMAVAULT_KDED_UI_NOTICE_WIDGET_H

#include "dialogdsl.h"

class NoticeWidget : public DialogDsl::DialogModule
{
    Q_OBJECT

public:
    enum Mode {
        ShowAlways,
        DoNotShowAgainOption,
    };

    NoticeWidget(const QString &noticeId, const QString &message, Mode mode);

    ~NoticeWidget() override;

    PlasmaVault::Vault::Payload fields() const override;

    void aboutToBeShown() override;
    bool shouldBeShown() const override;
    void aboutToBeHidden() override;

private:
    class Private;
    QScopedPointer<Private> d;
};

inline DialogDsl::ModuleFactory notice(const QByteArray &noticeId, const QString &message, NoticeWidget::Mode mode = NoticeWidget::DoNotShowAgainOption)
{
    return [=] {
        return new NoticeWidget(noticeId, message, mode);
    };
}

#endif // include guard
