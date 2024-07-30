/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <QQmlParserStatus>
#include <QUrl>
#include <QVariantList>

#include "potdengine.h"

/**
 * This class provides the backend of Pictures of The Day from various online
 * websites.
 */
class PotdBackend : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(QString identifier READ identifier WRITE setIdentifier NOTIFY identifierChanged)
    Q_PROPERTY(QVariantList arguments READ arguments WRITE setArguments NOTIFY argumentsChanged)

    /**
     * Read-only properties that expose data from the provider.
     */
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(QString localUrl READ localUrl NOTIFY localUrlChanged)
    /**
     * @return The website URL of the image
     */
    Q_PROPERTY(QUrl infoUrl READ infoUrl NOTIFY infoUrlChanged)
    /**
     * @return The remote image URL
     */
    Q_PROPERTY(QUrl remoteUrl READ remoteUrl NOTIFY remoteUrlChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(QString author READ author NOTIFY authorChanged)

    /**
     * @return the result of the file operation.
     */
    Q_PROPERTY(FileOperationStatus saveStatus MEMBER m_saveStatus NOTIFY saveStatusChanged)

    /**
     * @return the status message after a save operation.
     */
    Q_PROPERTY(QString saveStatusMessage MEMBER m_saveStatusMessage CONSTANT)

    /**
     * @return the folder path of the saved image file.
     */
    Q_PROPERTY(QUrl savedFolder MEMBER m_savedFolder CONSTANT)

    /**
     * @return the path of the saved image file.
     */
    Q_PROPERTY(QUrl savedUrl MEMBER m_savedUrl CONSTANT)

    /**
     * @return Any backend for \QNetworkInformation is available on the system
     */
    Q_PROPERTY(bool networkInformationAvailable MEMBER m_networkInfomationAvailable CONSTANT)

    /**
     * @return Whether to update wallpapers over metered connection
     */
    Q_PROPERTY(
        int updateOverMeteredConnection READ doesUpdateOverMeteredConnection WRITE setUpdateOverMeteredConnection NOTIFY updateOverMeteredConnectionChanged)

public:
    enum class FileOperationStatus {
        None,
        Successful,
        Failed,
    };
    Q_ENUM(FileOperationStatus)

    explicit PotdBackend(QObject *parent = nullptr);
    ~PotdBackend() override;

    void classBegin() override;
    void componentComplete() override;

    QString identifier() const;
    void setIdentifier(const QString &identifier);

    QVariantList arguments() const;
    void setArguments(const QVariantList &args);

    bool loading() const;
    QString localUrl() const;
    QUrl infoUrl() const;
    QUrl remoteUrl() const;
    QString title() const;
    QString author() const;

    int doesUpdateOverMeteredConnection() const;
    void setUpdateOverMeteredConnection(int value);

    /**
     * Opens a Save dialog to choose the save location, and copies the source file to the
     * selected destination.
     */
    Q_INVOKABLE void saveImage();

Q_SIGNALS:
    void identifierChanged();
    void argumentsChanged();

    void imageChanged();
    void loadingChanged();
    void localUrlChanged();
    void infoUrlChanged();
    void remoteUrlChanged();
    void titleChanged();
    void authorChanged();

    void saveStatusChanged();
    void updateOverMeteredConnectionChanged();

private:
    void registerClient();

    bool m_ready = false;
    bool m_networkInfomationAvailable = false;

    QString m_identifier;
    QVariantList m_args;

    QUrl m_savedFolder;
    QUrl m_savedUrl;
    FileOperationStatus m_saveStatus = FileOperationStatus::None;
    QString m_saveStatusMessage;

    int m_doesUpdateOverMeteredConnection = 0;

    PotdClient *m_client = nullptr;
};
