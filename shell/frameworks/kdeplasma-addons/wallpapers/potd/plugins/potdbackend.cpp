/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "potdbackend.h"

#include <QDBusConnection>
#include <QFileDialog>
#include <QNetworkInformation>
#include <QStandardPaths> // For "Pictures" folder

#include <KIO/CopyJob> // For "Save Image"
#include <KLocalizedString>

namespace
{
constinit PotdEngine *s_engine = nullptr;
constinit int s_instanceCount = 0;
}

PotdBackend::PotdBackend(QObject *parent)
    : QObject(parent)
    , m_networkInfomationAvailable(!QNetworkInformation::availableBackends().empty())
{
    if (!s_engine) {
        Q_ASSERT(s_instanceCount == 0);
        s_engine = new PotdEngine();
    }
    s_instanceCount++;
}

PotdBackend::~PotdBackend()
{
    s_engine->unregisterClient(m_identifier, m_args);
    s_instanceCount--;

    if (!s_instanceCount) {
        delete s_engine;
        s_engine = nullptr;
    }
}

void PotdBackend::classBegin()
{
}

void PotdBackend::componentComplete()
{
    // don't bother loading single image until all properties have settled
    m_ready = true;

    // Register the identifier in the data engine
    registerClient();
}

QString PotdBackend::identifier() const
{
    return m_identifier;
}

void PotdBackend::setIdentifier(const QString &identifier)
{
    if (m_identifier == identifier) {
        return;
    }

    if (m_ready) {
        s_engine->unregisterClient(m_identifier, m_args);
    }
    m_identifier = identifier;
    registerClient();

    Q_EMIT identifierChanged();
}

QVariantList PotdBackend::arguments() const
{
    return m_args;
}

void PotdBackend::setArguments(const QVariantList &args)
{
    if (m_args == args) {
        return;
    }

    if (m_ready) {
        s_engine->unregisterClient(m_identifier, m_args);
    }
    m_args = args;
    registerClient();

    Q_EMIT argumentsChanged();
}

bool PotdBackend::loading() const
{
    if (!m_client) {
        return false;
    }

    return m_client->m_loading;
}

QString PotdBackend::localUrl() const
{
    if (!m_client) {
        return {};
    }

    return m_client->m_localPath;
}

QUrl PotdBackend::infoUrl() const
{
    if (!m_client) {
        return {};
    }

    return m_client->m_infoUrl;
}

QUrl PotdBackend::remoteUrl() const
{
    if (!m_client) {
        return {};
    }

    return m_client->m_remoteUrl;
}

QString PotdBackend::title() const
{
    if (!m_client) {
        return {};
    }

    return m_client->m_title;
}

QString PotdBackend::author() const
{
    if (!m_client) {
        return {};
    }

    return m_client->m_author;
}

int PotdBackend::doesUpdateOverMeteredConnection() const
{
    return m_doesUpdateOverMeteredConnection;
}

void PotdBackend::setUpdateOverMeteredConnection(int value)
{
    value = std::clamp(value, 0, 2);
    const bool changed = m_doesUpdateOverMeteredConnection != value;
    if (changed) {
        m_doesUpdateOverMeteredConnection = value;
        Q_EMIT updateOverMeteredConnectionChanged();
    }

    if (m_ready && m_client) {
        m_client->setUpdateOverMeteredConnection(m_doesUpdateOverMeteredConnection);
    }
}

void PotdBackend::saveImage()
{
    if (m_client->m_localPath.isEmpty()) {
        return;
    }

    auto sanitizeFileName = [](const QString &name) {
        if (name.isEmpty()) {
            return name;
        }

        const char notAllowedChars[] = ",^@={}[]~!?:&*\"|#%<>$\"'();`'/\\";
        QString sanitizedName(name);

        for (const char *c = notAllowedChars; *c; c++) {
            sanitizedName.replace(QLatin1Char(*c), QLatin1Char('-'));
        }

        return sanitizedName;
    };

    const QStringList &locations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
    const QString path = locations.isEmpty() ? QStandardPaths::standardLocations(QStandardPaths::HomeLocation).at(0) : locations.at(0);

    QString defaultFileName = m_client->m_metadata.name().trimmed();

    if (!m_client->m_title.isEmpty()) {
        defaultFileName += QLatin1Char('-') + m_client->m_title.trimmed();
        if (!m_client->m_author.isEmpty()) {
            defaultFileName += QLatin1Char('-') + m_client->m_author.trimmed();
        }
    } else {
        // Use current date
        if (!defaultFileName.isEmpty()) {
            defaultFileName += QLatin1Char('-');
        }
        defaultFileName += QDate::currentDate().toString();
    }

    m_savedUrl = QUrl::fromLocalFile( //
        QFileDialog::getSaveFileName( //
            nullptr, //
            i18ndc("plasma_wallpaper_org.kde.potd", "@title:window", "Save Today's Picture"), //
            path + "/" + sanitizeFileName(defaultFileName) + ".jpg", //
            i18ndc("plasma_wallpaper_org.kde.potd", "@label:listbox Template for file dialog", "JPEG image (*.jpeg *.jpg *.jpe)"), //
            nullptr, //
            QFileDialog::DontConfirmOverwrite // KIO::CopyJob will show the confirmation dialog.
            ) //
    );

    if (m_savedUrl.isEmpty() || !m_savedUrl.isValid()) {
        return;
    }

    m_savedFolder = QUrl::fromLocalFile(QFileInfo(m_savedUrl.toLocalFile()).absolutePath());

    KIO::CopyJob *copyJob = KIO::copy(QUrl::fromLocalFile(m_client->m_localPath), m_savedUrl, KIO::HideProgressInfo);
    connect(copyJob, &KJob::finished, this, [this](KJob *job) {
        if (job->error()) {
            m_saveStatusMessage = job->errorText();
            if (m_saveStatusMessage.isEmpty()) {
                m_saveStatusMessage = i18ndc("plasma_wallpaper_org.kde.potd", "@info:status after a save action", "The image was not saved.");
            }
            m_saveStatus = FileOperationStatus::Failed;
            Q_EMIT saveStatusChanged();
        } else {
            m_saveStatusMessage = i18ndc("plasma_wallpaper_org.kde.potd",
                                         "@info:status after a save action %1 file path %2 basename",
                                         "The image was saved as <a href=\"%1\">%2</a>",
                                         m_savedUrl.toString(),
                                         m_savedUrl.fileName());
            m_saveStatus = FileOperationStatus::Successful;
            Q_EMIT saveStatusChanged();
        }
    });
    copyJob->start();
}

void PotdBackend::registerClient()
{
    if (!m_ready) {
        return;
    }

    m_client = s_engine->registerClient(m_identifier, m_args);

    if (!m_client) {
        // Invalid identifier
        return;
    }

    connect(m_client, &PotdClient::loadingChanged, this, &PotdBackend::loadingChanged);
    connect(m_client, &PotdClient::localUrlChanged, this, &PotdBackend::localUrlChanged);
    connect(m_client, &PotdClient::infoUrlChanged, this, &PotdBackend::infoUrlChanged);
    connect(m_client, &PotdClient::remoteUrlChanged, this, &PotdBackend::remoteUrlChanged);
    connect(m_client, &PotdClient::titleChanged, this, &PotdBackend::titleChanged);
    connect(m_client, &PotdClient::authorChanged, this, &PotdBackend::authorChanged);
    connect(m_client, &PotdClient::done, this, [this](PotdClient *, bool success) {
        if (success) {
            Q_EMIT imageChanged();
        }
    });

    // Refresh the desktop wallpaper and the information in config dialog
    Q_EMIT loadingChanged();
    Q_EMIT localUrlChanged();
    Q_EMIT infoUrlChanged();
    Q_EMIT remoteUrlChanged();
    Q_EMIT titleChanged();
    Q_EMIT authorChanged();

    // For updateSource()
    setUpdateOverMeteredConnection(m_doesUpdateOverMeteredConnection);
}
