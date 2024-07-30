/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2020 Ahmad Samir <a.samirh78@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kmessagedialog.h"

#include <KGuiItem>
#include <KStandardGuiItem>

#include <QApplication>
#include <QDebug>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QPushButton>

#include <memory>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("kmessagedialogtest"));

    const auto types = {KMessageDialog::QuestionTwoActions,
                        KMessageDialog::QuestionTwoActionsCancel,
                        KMessageDialog::WarningTwoActions,
                        KMessageDialog::WarningTwoActionsCancel,
                        KMessageDialog::WarningContinueCancel,
                        KMessageDialog::Information,
                        KMessageDialog::Error};

    for (auto type : types) {
        auto dlg = std::unique_ptr<KMessageDialog>(new KMessageDialog(type, QStringLiteral("Message in a box."), nullptr));
        dlg->setCaption(QString{});
        dlg->setIcon(QIcon{});
        if ((type != KMessageDialog::Information) && (type != KMessageDialog::Error)) {
            dlg->setButtons(KGuiItem(QStringLiteral("Primary Action")), KGuiItem(QStringLiteral("Secondary Action")), KStandardGuiItem::cancel());
        }

        auto getResult = [&]() {
            const auto result = static_cast<KMessageDialog::ButtonType>(dlg->exec());
            switch (result) {
            case KMessageDialog::Ok:
            case KMessageDialog::PrimaryAction:
                qDebug() << "Button OK/Primary clicked."
                         << "Don't ask again status:" << (dlg->isDontAskAgainChecked() ? "Checked" : "Unchecked");
                break;
            case KMessageDialog::SecondaryAction:
                qDebug() << "Button Secondary clicked"
                         << "Don't ask again status:" << (dlg->isDontAskAgainChecked() ? "Checked" : "Unchecked");
                break;
            case KMessageDialog::Cancel:
                qDebug() << "Button Cancel clicked";
                break;
            default:
                break;
            }
        };

        getResult();

        dlg->setDontAskAgainText(QStringLiteral("Do not show this again"));
        dlg->setDontAskAgainChecked(false);
        getResult();

        dlg->setListWidgetItems(QStringList{QStringLiteral("file1"), QStringLiteral("file2")});
        getResult();

        dlg->setDetails(QStringLiteral("Some more details."));
        getResult();
    }
    return app.exec();
}
