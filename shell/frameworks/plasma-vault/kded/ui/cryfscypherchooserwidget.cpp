/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "cryfscypherchooserwidget.h"

#include "ui_cryfscypherchooserwidget.h"

#include <QProcess>
#include <QStringList>
#include <QTimer>

#include <asynqt/basic/all.h>
#include <asynqt/operations/transform.h>
#include <asynqt/wrappers/process.h>

class CryfsCypherChooserWidget::Private
{
public:
    Ui::CryfsCypherChooserWidget ui;
};

CryfsCypherChooserWidget::CryfsCypherChooserWidget()
    : DialogDsl::DialogModule(true)
    , d(new Private())
{
    d->ui.setupUi(this);

    QTimer::singleShot(0, this, &CryfsCypherChooserWidget::initializeCyphers);
}

void CryfsCypherChooserWidget::initializeCyphers()
{
    using namespace AsynQt;
    using namespace AsynQt::operators;

    // TODO: This needs to be prettier -- for modules to be able
    // to reach their backends directly
    auto process = new QProcess();
    process->setProgram("cryfs");
    process->setArguments({"--show-ciphers"});

    auto env = process->processEnvironment();
    env.insert("CRYFS_FRONTEND", "noninteractive");
    process->setProcessEnvironment(env);

    auto combo = d->ui.comboCypher;

    process->start();

    while (!process->waitForFinished(10)) {
        QCoreApplication::processEvents();
    }

    const auto err = process->readAllStandardError();

    combo->addItem(i18n("Use the default cipher"), QString());

    for (const auto &item : QString::fromLatin1(err).split('\n')) {
        if (!item.isEmpty()) {
            combo->addItem(item, item);
        }
    }
}

CryfsCypherChooserWidget::~CryfsCypherChooserWidget()
{
}

PlasmaVault::Vault::Payload CryfsCypherChooserWidget::fields() const
{
    return {{"cryfs-cipher", d->ui.comboCypher->currentData()}};
}

#include "moc_cryfscypherchooserwidget.cpp"
