/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2012 David Faure <faure+bluesystems@kde.org>
    SPDX-FileCopyrightText: 2013 Aurélien Gâteau <agateau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "kmessagebox_p.h"

#include "loggingcategory.h"

#include <QCoreApplication>
#include <QPluginLoader>
#include <QSettings>
#include <QStandardPaths>

namespace KMessageBox
{
class KMessageBoxDontAskAgainQSettingsStorage : public KMessageBoxDontAskAgainInterface
{
public:
    KMessageBoxDontAskAgainQSettingsStorage()
    {
        m_filePath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1Char('/') + QCoreApplication::instance()->applicationName()
            + QLatin1String(".kmessagebox");
        QSettings s(m_filePath, QSettings::IniFormat);
        const auto keys = s.allKeys();
        for (const QString &key : keys) {
            m_saved.insert(key, static_cast<KMessageBox::ButtonCode>(s.value(key).toInt()));
        }
    }

    ~KMessageBoxDontAskAgainQSettingsStorage() override
    {
        QSettings s(m_filePath, QSettings::IniFormat);
        for (auto it = m_saved.constBegin(); it != m_saved.constEnd(); ++it) {
            s.setValue(it.key(), static_cast<int>(it.value()));
        }
    }

    bool shouldBeShownTwoActions(const QString &dontShowAgainName, KMessageBox::ButtonCode &result) override
    {
        KMessageBox::ButtonCode code = m_saved.value(dontShowAgainName, KMessageBox::ButtonCode(0));
        if (code == KMessageBox::PrimaryAction || code == KMessageBox::SecondaryAction) {
            result = code;
            return false;
        }
        return true;
    }
    bool shouldBeShownContinue(const QString &dontShowAgainName) override
    {
        KMessageBox::ButtonCode code = m_saved.value(dontShowAgainName, KMessageBox::PrimaryAction);
        return code == KMessageBox::PrimaryAction;
    }
    void saveDontShowAgainTwoActions(const QString &dontShowAgainName, KMessageBox::ButtonCode result) override
    {
        m_saved[dontShowAgainName] = result;
    }
    void saveDontShowAgainContinue(const QString &dontShowAgainName) override
    {
        m_saved[dontShowAgainName] = KMessageBox::SecondaryAction;
    }
    void enableAllMessages() override
    {
        m_saved.clear();
    }
    void enableMessage(const QString &dontShowAgainName) override
    {
        m_saved.remove(dontShowAgainName);
    }
    void setConfig(KConfig *) override
    {
        qCWarning(KWidgetsAddonsLog) << "Using QSettings based KMessageBoxDontAskAgainInterface. KMessageBox::setDontShowAgainConfig ignored";
    }

private:
    QString m_filePath;
    QHash<QString, KMessageBox::ButtonCode> m_saved;
};

class KMessageBoxNotifyDummy : public KMessageBoxNotifyInterface
{
public:
    void sendNotification(QMessageBox::Icon /*notificationType*/, const QString & /*message*/, QWidget * /*parent*/) override
    {
    }
};

Q_GLOBAL_STATIC(KMessageBoxDontAskAgainQSettingsStorage, s_defaultDontAskAgainInterface)
Q_GLOBAL_STATIC(KMessageBoxNotifyDummy, s_defaultNotifyInterface)

static KMessageBoxDontAskAgainInterface *s_dontAskAgainInterface = nullptr;
static KMessageBoxNotifyInterface *s_notifyInterface = nullptr;

static void loadKMessageBoxPlugin()
{
    static bool triedLoadingPlugin = false;
    if (!triedLoadingPlugin) {
        triedLoadingPlugin = true;

        QPluginLoader lib(QStringLiteral("kf6/FrameworkIntegrationPlugin"));
        QObject *rootObj = lib.instance();
        if (rootObj) {
            s_dontAskAgainInterface = rootObj->property(KMESSAGEBOXDONTASKAGAIN_PROPERTY).value<KMessageBoxDontAskAgainInterface *>();
            s_notifyInterface = rootObj->property(KMESSAGEBOXNOTIFY_PROPERTY).value<KMessageBoxNotifyInterface *>();
        }
    }
    if (!s_dontAskAgainInterface) {
        s_dontAskAgainInterface = s_defaultDontAskAgainInterface;
    }
    if (!s_notifyInterface) {
        s_notifyInterface = s_defaultNotifyInterface;
    }
}

KMessageBoxDontAskAgainInterface *dontAskAgainInterface()
{
    if (!s_dontAskAgainInterface) {
        loadKMessageBoxPlugin();
    }
    return s_dontAskAgainInterface;
}

KMessageBoxNotifyInterface *notifyInterface()
{
    if (!s_notifyInterface) {
        loadKMessageBoxPlugin();
    }
    return s_notifyInterface;
}

bool isNotifyInterfaceLoaded()
{
    return s_notifyInterface;
}

void setDontShowAgainInterface(KMessageBoxDontAskAgainInterface *dontAskAgainInterface)
{
    Q_ASSERT(dontAskAgainInterface != nullptr);
    s_dontAskAgainInterface = dontAskAgainInterface;
}

void setNotifyInterface(KMessageBoxNotifyInterface *notifyInterface)
{
    Q_ASSERT(notifyInterface != nullptr);
    s_notifyInterface = notifyInterface;
}

} // KMessageBox namespace
