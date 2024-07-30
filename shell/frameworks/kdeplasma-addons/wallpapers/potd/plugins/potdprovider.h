// SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
// SPDX-FileCopyrightText: 2021 Guo Yunhe <i@guoyunhe.me>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QImage>
#include <QUrl>
#include <QVariantList>

#include <KPluginMetaData>

#include "plasma_potd_export.h"

class QDate;

class PotdProviderPrivate;

/**
 * This class is an interface for PoTD providers.
 */
class PLASMA_POTD_EXPORT PotdProvider : public QObject
{
    Q_OBJECT

public:
    /**
     * Creates a new PoTD provider.
     *
     * @param parent The parent object.
     * @param data The metadata of the plugin
     * @param args The arguments.
     * @since 5.25
     */
    explicit PotdProvider(QObject *parent, const KPluginMetaData &data, const QVariantList &args);

    /**
     * Destroys the PoTD provider.
     */
    virtual ~PotdProvider() override;

    /**
     * Returns the local path of the requested image.
     *
     * Note: This method returns only a valid path after the
     *       finished() signal has been emitted.
     */
    virtual QString localPath() const;

    /**
     * Returns the identifier of the PoTD request (name + date).
     */
    virtual QString identifier() const;

    /**
     * Returns the remote URL of the image from the provider
     *
     * @note No @c virtual to keep binary compatibility.
     * @return the remote URL of the image, if any
     * @since 5.25
     */
    QUrl remoteUrl() const;

    /**
     * Returns the information URL of the image from the provider
     *
     * @return the information URL of the image, if any
     * @since 5.25
     */
    QUrl infoUrl() const;

    /**
     * Returns the title of the image from the provider, if any.
     *
     * @return the title of the image, if any
     * @since 5.25
     */
    QString title() const;

    /**
     * Returns the author of the image from the provider
     *
     * @return the title of the image, if any
     * @since 5.25
     */
    QString author() const;

    /**
     * @return the name of this provider (equiv to X-KDE-PlasmaPoTDProvider-Identifier)
     */
    QString name() const;

Q_SIGNALS:
    /**
     * This signal is emitted whenever a request has been finished
     * successfully.
     *
     * @param provider The provider which emitted the signal.
     * @param image The image from the provider.
     */
    void finished(PotdProvider *provider, const QImage &image);

    /**
     * This signal is emitted whenever an error has occurred.
     *
     * @param provider The provider which emitted the signal.
     */
    void error(PotdProvider *provider);

protected:
    QUrl m_remoteUrl;
    QUrl m_infoUrl;
    QString m_title;
    QString m_author;

private:
    std::unique_ptr<PotdProviderPrivate> d;
};
