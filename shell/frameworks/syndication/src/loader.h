/*
    loader.h
    SPDX-FileCopyrightText: 2001, 2002, 2003 Frerich Raabe <raabe@kde.org>

    SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef SYNDICATION_LOADER_H
#define SYNDICATION_LOADER_H

#include "global.h"

#include "syndication_export.h"

#include <QObject>

#include <memory>

class QUrl;

namespace Syndication
{
class DataRetriever;
class Feed;
//@cond PRIVATE
typedef QSharedPointer<Feed> FeedPtr;
//@endcond

/**
 * This class is the preferred way of loading feed sources. Usage is very
 * straightforward:
 *
 * \code
 * Loader *loader = Loader::create();
 * connect(loader, SIGNAL(loadingComplete(Loader*, FeedPtr, ErrorCode)),
 *         this, SLOT(slotLoadingComplete(Loader*, FeedPtr, ErrorCode)));
 * loader->loadFrom("http://www.blah.org/foobar.rdf");
 * \endcode
 *
 * This creates a Loader object, connects it's loadingComplete() signal to
 * your custom slot and then makes it load the file
 * 'http://www.blah.org/foobar.rdf'. You could've
 * done something like this as well:
 *
 * \code
 * // create the Loader, connect it's signal...
 * loader->loadFrom("/home/myself/some-script.py", new OutputRetriever);
 * \endcode
 *
 * That'd make the Loader use a custom algorithm for retrieving the RSS data;
 * 'OutputRetriever' will make it execute the script
 * '/home/myself/some-script.py' and assume whatever that script prints to
 * stdout is RSS/Azom markup. This is e.g. handy for conversion scripts, which
 * download a HTML file and convert it's contents into RSS markup.
 *
 * No matter what kind of retrieval algorithm you employ, your
 * 'slotLoadingComplete' method might look like this:
 *
 * \code
 * void MyClass::slotLoadingComplete(Loader* loader, FeedPtr feed, ErrorCode status)
 * {
 *     // Note that Loader::~Loader() is private, so you cannot delete Loader instances.
 *     // You don't need to do that anyway since Loader instances delete themselves.
 *
 *     if (status != Syndication::Success)
 *         return;
 *
 *     QString title = feed->title();
 *     // do whatever you want with the information.
 * }
 * \endcode
 */
class SYNDICATION_EXPORT Loader : public QObject
{
    Q_OBJECT

public:
    /**
     * Constructs a Loader instance. This is pretty much what the
     * default constructor would do, except that it ensures that all
     * Loader instances have been allocated on the heap (this is
     * required so that Loader's can delete themselves safely after they
     * emitted the loadingComplete() signal.).
     * @return A pointer to a new Loader instance.
     */
    static Loader *create();

    /**
     * Convenience method. Does the same as the above method except that
     * it also does the job of connecting the loadingComplete() signal
     * to the given slot for you.
     * @param object A QObject which features the specified slot
     * @param slot Which slot to connect to.
     */
    static Loader *create(QObject *object, const char *slot);

    /**
     * Loads the feed source referenced by the given URL using the
     * specified retrieval algorithm. Make sure that you connected
     * to the loadingComplete() signal before calling this method so
     * that you're guaranteed to get notified when the loading finished.
     * \note A Loader object cannot load from multiple URLs simultaneously;
     * consequently, subsequent calls to loadFrom will be discarded
     * silently, only the first loadFrom request will be executed.
     * @param url A URL referencing the input file.
     * @param retriever A subclass of DataRetriever which implements a
     * specialized retrieval behaviour. Note that the ownership of the
     * retriever is transferred to the Loader, i.e. the Loader will
     * delete it when it doesn't need it anymore.
     * @see DataRetriever, Loader::loadingComplete()
     */
    void loadFrom(const QUrl &url, DataRetriever *retriever);

    /**
     * Retrieves the error code of the last loading process (if any).
     */
    Q_REQUIRED_RESULT ErrorCode errorCode() const;

    /**
     * the error code returned from the retriever.
     * Use this if you use your custom retriever implementation and
     * need the specific error, not covered by errorCode().
     */
    Q_REQUIRED_RESULT int retrieverError() const;

    /**
     * returns the URL of a feed discovered in the feed source
     */
    Q_REQUIRED_RESULT QUrl discoveredFeedURL() const;

    /**
     * aborts the loading process
     */
    void abort();

Q_SIGNALS:

    /**
     * This signal gets emitted when the loading process triggered by
     * calling loadFrom() finished.
     * @param loader A pointer pointing to the loader object which
     * emitted this signal; this is handy in case you connect multiple
     * loaders to a single slot.
     * @param feed In case errortus is Success, this parameter holds the
     * parsed feed. If fetching/parsing failed, feed is NULL.
     * @param error An error code telling whether there were any
     * problems while retrieving or parsing the data.
     * @see Feed, ErrorCode
     */
    void loadingComplete(Syndication::Loader *loader, Syndication::FeedPtr feed, Syndication::ErrorCode error);

private Q_SLOTS:
    SYNDICATION_NO_EXPORT void slotRetrieverDone(const QByteArray &data, bool success);

private:
    SYNDICATION_NO_EXPORT Loader();
    Loader(const Loader &other);
    Loader &operator=(const Loader &other);
    SYNDICATION_NO_EXPORT ~Loader() override;
    SYNDICATION_NO_EXPORT void discoverFeeds(const QByteArray &data);

    struct LoaderPrivate;
    std::unique_ptr<LoaderPrivate> const d;
};

} // namespace Syndication

#endif // SYNDICATION_LOADER_H
