// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "appchooser.h"
#include "appchooserdialog.h"
#include "request.h"
#include "utils.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusServiceWatcher>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(appChooserCategory, "xdg-dde-appchooser")
AppChooserPortal::AppChooserPortal(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
#ifdef QT_DEBUG
    QVariantMap results;
    QVariantMap options;
    options.insert("modal", true);
    options.insert("content_type", "title");
    ChooseApplication(QDBusObjectPath(), "", "", QStringList(), options, results);
    qDebug() << results;
#endif
}

AppChooserPortal::~AppChooserPortal()
{
}

uint AppChooserPortal::ChooseApplication(const QDBusObjectPath &handle, const QString &app_id, const QString &parent_window, const QStringList &choices, const QVariantMap &options, QVariantMap &results)
{
    qCDebug(appChooserCategory) << __FUNCTION__ << "args:"
                                << "\n handle:" << handle.path()
                                << "\n app_id:" << app_id
                                << "\n parent_window:" << parent_window
                                << "\n choices:" << choices
                                << "\n options:" << options;

    Q_UNUSED(choices);
    // The app id that was selected the last time.
    DECLEAR_PARA(last_choice, toString);
    // Whether to make the dialog modal. Defaults to yes.
    DECLEAR_PARA_WITH_FALLBACK(modal, toBool, true);
    // The content type to choose an application for.
    DECLEAR_PARA(content_type, toString);
    // The uri to choose an application for.
    DECLEAR_PARA(uri, toString);
    // The filename to choose an application for. Note that this is just a basename, without a path.
    DECLEAR_PARA(filename, toString);
    // A token that can be used to activate the application chooser.
    // The activation_token option was introduced in version 2 of the interface.
    DECLEAR_PARA(activation_token, toString);
    Q_UNUSED(activation_token)

    AppChooserDialog *dialog = new AppChooserDialog;
    dialog->setModal(modal);
    dialog->setWindowTitle(!content_type.isEmpty() ? content_type : (!uri.isEmpty() ? uri : filename));
    dialog->setCurrentChoice(last_choice);

    m_appChooserDialogs.insert(handle.path(), dialog);
    Utils::setParentWindow(dialog->windowHandle(), parent_window);

    auto request = new Request(handle, QVariant(), dialog);
    connect(request, &Request::closeRequested, dialog, &AppChooserDialog::reject);

    int result = dialog->exec();

    if (QDialog::Accepted == result) {
        results.insert(QStringLiteral("choice"), dialog->selectChoices());
    }

    m_appChooserDialogs.remove(handle.path());
    dialog->deleteLater();

    return !result;
}

void AppChooserPortal::UpdateChoices(const QDBusObjectPath &handle, const QStringList &choices)
{
    qCDebug(appChooserCategory) << __FUNCTION__ << "args:"
                                << "\n handle:" << handle.path()
                                << "\n choices:" << choices;

    if (m_appChooserDialogs.contains(handle.path())) {
        m_appChooserDialogs.value(handle.path())->updateChoices(choices);
    }
}
