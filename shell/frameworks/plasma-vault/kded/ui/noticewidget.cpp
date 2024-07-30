/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "noticewidget.h"

#include "ui_noticewidget.h"

#include <KConfigGroup>
#include <KSharedConfig>

class NoticeWidget::Private
{
public:
    Ui::NoticeWidget ui;
    KSharedConfig::Ptr config;
    bool shouldBeShown;
    QString noticeId;
};

NoticeWidget::NoticeWidget(const QString &noticeId, const QString &message, Mode mode)
    : DialogDsl::DialogModule(true)
    , d(new Private())
{
    d->ui.setupUi(this);
    d->ui.textNotice->setHtml(message);
    d->ui.checkShouldBeHidden->setVisible(mode == DoNotShowAgainOption);

    d->noticeId = noticeId;

    d->config = KSharedConfig::openConfig(PLASMAVAULT_CONFIG_FILE);
}

NoticeWidget::~NoticeWidget()
{
}

PlasmaVault::Vault::Payload NoticeWidget::fields() const
{
    return {};
}

void NoticeWidget::aboutToBeShown()
{
    KConfigGroup noticeUi(d->config, "UI-notice");
    d->shouldBeShown = !noticeUi.readEntry("SkipNotice-" + d->noticeId, false);
    d->ui.checkShouldBeHidden->setChecked(!d->shouldBeShown);
}

bool NoticeWidget::shouldBeShown() const
{
    return d->shouldBeShown;
}

void NoticeWidget::aboutToBeHidden()
{
    KConfigGroup noticeUi(d->config, "UI-notice");
    noticeUi.writeEntry("SkipNotice-" + d->noticeId, d->ui.checkShouldBeHidden->isChecked());
    d->config->sync();
}

#include "moc_noticewidget.cpp"
