/*
    SPDX-FileCopyrightText: 2004 Aaron J. Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "button.h"

#include "dialog.h"
#include <KAuthorized>
#include <KLocalizedString>
#include <KMessageBox>

#include <QPointer>

namespace KNSWidgets
{
class ButtonPrivate
{
public:
    explicit ButtonPrivate(Button *qq)
        : q(qq)
    {
    }

    void showDialog()
    {
        if (!KAuthorized::authorize(KAuthorized::GHNS)) {
            KMessageBox::information(q, QStringLiteral("Get Hot New Stuff is disabled by the administrator"), QStringLiteral("Get Hot New Stuff disabled"));
            return;
        }
        Q_ASSERT_X(!configFile.isEmpty(), Q_FUNC_INFO, "The configFile for the KNSWidgets::Button must be explicitly set");

        if (!dialog) {
            dialog.reset(new KNSWidgets::Dialog(configFile, q));
            dialog->setWindowTitle(q->text().remove(QLatin1Char('&')));
            QObject::connect(dialog.get(), &KNSWidgets::Dialog::finished, q, [this]() {
                Q_EMIT q->dialogFinished(dialog->changedEntries());
            });
        }
        dialog->open();
    }

    Button *q;
    QString configFile;
    std::unique_ptr<KNSWidgets::Dialog> dialog;
};

Button::Button(const QString &text, const QString &configFile, QWidget *parent)
    : QPushButton(parent)
    , d(new ButtonPrivate(this))
{
    setText(text);
    d->configFile = configFile;

    const bool authorized = KAuthorized::authorize(KAuthorized::GHNS);
    if (!authorized) {
        setEnabled(false);
        setVisible(false);
    }

    setIcon(QIcon::fromTheme(QStringLiteral("get-hot-new-stuff")));
    connect(this, &QAbstractButton::clicked, this, [this]() {
        d->showDialog();
    });
}

Button::Button(QWidget *parent)
    : Button(i18nc("@action", "Download New Stuff…"), QString(), parent)
{
}

Button::~Button() = default;

void Button::setConfigFile(const QString &configFile)
{
    Q_ASSERT_X(!d->dialog, Q_FUNC_INFO, "the configFile property must be set before the dialog is first shown");
    d->configFile = configFile;
}
}

#include "moc_button.cpp"
