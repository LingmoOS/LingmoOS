/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *   SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
 *
 *   SPDX-License-Identifier: LGPL-2.0-only
 */

#ifndef COMICPROVIDER_H
#define COMICPROVIDER_H

#include <KPluginMetaData>
#include <QDate>
#include <QObject>

#include "types.h"

class QImage;
class QUrl;

/**
 * This class is an interface for comic providers.
 */
class ComicProvider : public QObject
{
    Q_OBJECT

public:
    enum RequestType {
        Page = 0,
        Image,
        User,
    };

    /**
     * Creates a new comic provider.
     *
     * @param parent The parent object.
     * @param args Arguments passed by the plugin loader.
     */
    ComicProvider(QObject *parent, const KPluginMetaData &data, IdentifierType type, const QVariant &identifier);

    /**
     * Destroys the comic provider.
     */
    ~ComicProvider() override;

    /**
     * Returns the type of identifier that is used by this
     * comic provider.
     */
    virtual IdentifierType identifierType() const = 0;

    /**
     * Returns the url of the website where the comic of that particular date resides.
     */
    virtual QUrl websiteUrl() const = 0;

    /**
     * Returns the direct url to the comic, if the comic strip is a combination of multiple
     * images, then this should return the url to one part of it
     * @note the image url is automatically set by requestPage with the ComicProvider::Image id
     * @see requestPage
     */
    virtual QUrl imageUrl() const;

    /**
     * Returns the url of the website where the comic has a shop.
     */
    virtual QUrl shopUrl() const;

    /**
     * Returns the requested image.
     *
     * Note: This method returns only a valid image after the
     *       finished() signal has been emitted.
     */
    virtual QImage image() const = 0;

    /**
     * Returns the identifier of the comic request.
     */
    virtual QString identifier() const = 0;

    /**
     * Returns the identifier of the next comic (default: date of next day).
     */
    virtual QString nextIdentifier() const;

    /**
     * Returns the identifier of the previous comic (default: date of previous day
     * as long).
     */
    virtual QString previousIdentifier() const;

    /**
     * Returns the identifier of the first strip.
     */
    virtual QString firstStripIdentifier() const;

    /**
     * Returns the author of the comic.
     */
    virtual QString comicAuthor() const;

    /**
     * Returns the title of the strip.
     */
    virtual QString stripTitle() const;

    /**
     * Returns additionalText of the comic.
     */
    virtual QString additionalText() const;

    /**
     * Returns the identifier for the comic
     */
    virtual QString pluginName() const;

    /**
     * Returns the name for the comic
     */
    virtual QString name() const;

    /**
     * Returns whether the comic is leftToRight or not
     */
    virtual bool isLeftToRight() const;

    /**
     * Returns whether the comic is topToBottom or not
     */
    virtual bool isTopToBottom() const;

    /**
     * Returns the plugin info for the comic
     */
    KPluginMetaData description() const;

    /**
     * Set whether this request is for the current comic (only used internally).
     */
    void setIsCurrent(bool value);

    /**
     * Returns whether this request is for the current comic (only used internally).
     */
    bool isCurrent() const;

Q_SIGNALS:
    /**
     * This signal is emitted whenever a request has been finished
     * successfully.
     *
     * @param provider The provider which emitted the signal.
     */
    void finished(ComicProvider *provider);

    /**
     * This signal is emitted whenever an error has occurred.
     *
     * @param provider The provider which emitted the signal.
     */
    void error(ComicProvider *provider);

protected:
    /**
     * Returns the date identifier that was requested by the applet.
     */
    QDate requestedDate() const;

    /**
     * Returns the numeric identifier that was requested by the applet.
     */
    int requestedNumber() const;

    /**
     * Returns the string identifier that was requested by the applet.
     */
    QString requestedString() const;

    /**
     * @internal
     *
     * Returns the comic name of the string identifier that was requested by the applet.
     */
    QString requestedComicName() const;

    /**
     * Returns the date of the first available comic strip.
     */
    QDate firstStripDate() const;

    /**
     * Sets the date of the first available comic strip.
     */
    void setFirstStripDate(const QDate &date);

    /**
     * Returns the number of the first available comic strip (default: 1).
     */
    int firstStripNumber() const;

    /**
     * Sets the number of the first available comic strip.
     */
    void setFirstStripNumber(int number);

    /**
     * Sets the name of the comic author.
     */
    void setComicAuthor(const QString &author);

    typedef QMap<QString, QString> MetaInfos;

    /**
     * This method should be used by all comic providers to request
     * websites or images from the web. It encapsulates the HTTP
     * handling and calls pageRetrieved() or pageError() on success or error.
     *
     * @param url The url to access.
     * @param id A unique id that identifies this request.
     * @param infos A list of meta information passed to http.
     */
    void requestPage(const QUrl &url, int id, const MetaInfos &infos = MetaInfos());

    /**
     * This method can be used to find the place url points to, when finished
     * urlRetrieved() is called, either with the original url or a redirected url
     * @param url to check for redirections
     * @param id A unique id that identifies this request.
     * @param infos A list of meta information passed to KIO.
     */
    void requestRedirectedUrl(const QUrl &url, int id, const MetaInfos &infos = MetaInfos());

    /**
     * This method is called whenever a request done by requestPage() was successful.
     *
     * @param id The unique identifier of that request.
     * @param data The data of the fetched object.
     */
    virtual void pageRetrieved(int id, const QByteArray &data);

    /**
     * This method is called whenever a request done by requestPage() has failed.
     *
     * @param id The unique identifier of that request.
     * @param message The error message.
     */
    virtual void pageError(int id, const QString &message);

    /**
     * This method is called whenever a request by requestRedirectedUrl() was done
     * @param id The unique identifier of that request.
     * @param newUrl The redirected Url
     */
    virtual void redirected(int id, const QUrl &newUrl);

private:
    class Private;
    Private *const d;
};

#endif
