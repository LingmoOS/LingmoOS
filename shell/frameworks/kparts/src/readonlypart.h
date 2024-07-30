/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 1999 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _KPARTS_READONLYPART_H
#define _KPARTS_READONLYPART_H

#include <kparts/part.h>

#include <QUrl>

class KJob;
namespace KIO
{
class Job;
}

namespace KParts
{
class ReadOnlyPartPrivate;
class NavigationExtension;
class OpenUrlArguments;

/**
 * @class ReadOnlyPart readonlypart.h <KParts/ReadOnlyPart>
 *
 * @short Base class for any "viewer" part.
 *
 * This class takes care of network transparency for you,
 * in the simplest way (downloading to a temporary file, then letting the part
 * load from the temporary file).
 * To use the built-in network transparency, you only need to implement
 * openFile(), not openUrl().
 *
 * To implement network transparency differently (e.g. for progressive loading,
 * like a web browser does for instance), or to prevent network transparency
 * (but why would you do that?), you can override openUrl().
 *
 * An application using KParts can use the signals to show feedback while
 * the URL is being loaded.
 *
 * ReadOnlyPart handles the window caption by setting it to the current URL
 * (set in openUrl(), and each time the part is activated).
 * If you want another caption, set it in openFile() and,
 * if the part might ever be used with a part manager, in guiActivateEvent().
 */
class KPARTS_EXPORT ReadOnlyPart : public Part
{
    Q_OBJECT

    Q_PROPERTY(QUrl url READ url)

    KPARTS_DECLARE_PRIVATE(ReadOnlyPart)

public:
    /**
     * Constructor.
     * See also Part for the setXXX methods to call.
     */
    explicit ReadOnlyPart(QObject *parent = nullptr, const KPluginMetaData &data = {});

    /**
     * Destructor.
     */
    ~ReadOnlyPart() override;

    /**
     * Call this to turn off the progress info dialog used by
     * the internal KIO job. Use this if you provide another way
     * of displaying progress info (e.g. a statusbar), using the
     * signals emitted by this class, and/or those emitted by
     * the job given by started().
     */
    void setProgressInfoEnabled(bool show);

    /**
     * Returns whether the part shows the progress info dialog used by the internal
     * KIO job.
     */
    bool isProgressInfoEnabled() const;

public Q_SLOTS:
    /**
     * Only reimplement this if you don't want the network transparency support
     * to download from the URL into a temporary file (when the URL isn't local).
     * Otherwise, reimplement openFile() only.
     *
     * If you reimplement it, don't forget to set the caption, usually with
     * @code
     * Q_EMIT setWindowCaption( url.toDisplayString(QUrl::PreferLocalFile) );
     * @endcode
     * and also, if the URL refers to a local file, resolve it to a
     * local path and call setLocalFilePath().
     */
    virtual bool openUrl(const QUrl &url);

public:
    /**
     * Returns the URL currently opened in (or being opened by) this part.
     * @note The URL is not cleared if openUrl() fails to load the URL.
     *       Call closeUrl() if you need to explicitly reset it.
     *
     *  @return The current URL.
     */
    QUrl url() const;

    /**
     * Called when closing the current URL (for example, a document), for instance
     * when switching to another URL (note that openUrl() calls it
     * automatically in this case).
     * If the current URL is not fully loaded yet, aborts loading.
     * Deletes the temporary file used when the URL is remote.
     * Resets the current url() to QUrl().
     * @return always true, but the return value exists for reimplementations
     */
    virtual bool closeUrl();

    /**
     * This convenience method returns the NavigationExtension for this part,
     * or @c nullptr if there isn't one.
     */
    NavigationExtension *navigationExtension() const;

    /**
     * Sets the arguments to use for the next openUrl() call.
     */
    void setArguments(const OpenUrlArguments &arguments);
    // TODO to avoid problems with the case where the loading fails, this could also be a openUrl() argument (heavy porting!).
    // However we need to have setArguments in any case for updated made by the part, see e.g. KHTMLPart::openUrl.
    // Well, maybe we should have setArguments (affects next openurl call) and updateArguments?

    /**
     * @return the arguments that were used to open this URL.
     */
    OpenUrlArguments arguments() const;

public:
    /**
     * Initiate sending data to this part.
     * This is an alternative to openUrl(), which allows the user of the part
     * to load the data itself, and send it progressively to the part.
     *
     * @param mimeType the type of data that is going to be sent to this part.
     * @param url the URL representing this data. Although not directly used,
     * every ReadOnlyPart has a URL (see url()), so this simply sets it.
     * @return true if the part supports progressive loading and accepts data, false otherwise.
     */
    bool openStream(const QString &mimeType, const QUrl &url);

    /**
     * Send some data to the part. openStream must have been called previously,
     * and must have returned true.
     * @return true if the data was accepted by the part. If false is returned,
     * the application should stop sending data, and doesn't have to call closeStream.
     */
    bool writeStream(const QByteArray &data);

    /**
     * Terminate the sending of data to the part.
     * With some data types (text, html...) closeStream might never actually be called,
     * in the case of continuous streams, for instance plain text or HTML data.
     */
    bool closeStream();

#ifdef K_DOXYGEN
protected: // are parsed by doxygen (kapidox/ecm_add_qch): unhide for doxygen configured to skip private methods
#else
private: // Makes no sense for inherited classes to call those. But make it protected there.
#endif // K_DOXYGEN

    /**
     * Called by openStream to initiate sending of data.
     * Parts which implement progress loading should check the @p mimeType
     * parameter, and return true if they can accept a data stream of that type.
     */
    virtual bool doOpenStream(const QString &mimeType)
    {
        Q_UNUSED(mimeType);
        return false;
    }
    /**
     * Receive some data from the hosting application.
     * In this method the part should attempt to display the data progressively.
     * With some data types (text, html...) closeStream might never actually be called,
     * in the case of continuous streams. This can't happen with e.g. images.
     */
    virtual bool doWriteStream(const QByteArray &data)
    {
        Q_UNUSED(data);
        return false;
    }
    /**
     * This is called by closeStream(), to indicate that all the data has been sent.
     * Parts should ensure that all of the data is displayed at this point.
     * @return whether the data could be displayed correctly.
     */
    virtual bool doCloseStream()
    {
        return false;
    }

Q_SIGNALS:
    /**
     * The part emits this when starting to load data.
     * If using a KIO::Job, it provides the @p job so that
     * progress information can be shown. Otherwise, @p job is @c nullptr.
     **/
    void started(KIO::Job *job);

    /**
     * Emit this when you have completed loading data.
     * Hosting applications will want to know when the process of loading the data
     * is finished, so that they can access the data when everything is loaded.
     **/
    void completed();

    /**
     * This signal is similar to the @c KParts::ReadOnlyPart::completed() signal
     * except it is only emitted if there is still a pending action to be executed
     * on a delayed timer.
     *
     * An example of this is the meta-refresh tags on web pages used to reload/redirect
     * after a certain period of time. This signal is useful if you want to give the
     * user the ability to cancel such pending actions.
     *
     * @since 5.81
     */
    void completedWithPendingAction();

    /**
     * Emit this if loading is canceled by the user or by an error.
     * @param errMsg the error message, empty if the user canceled the loading voluntarily.
     */
    void canceled(const QString &errMsg);

    /**
     * Emitted by the part when url() changes
     * @since 4.10
     */
    void urlChanged(const QUrl &url);

protected:
    /**
     * If the part uses the standard implementation of openUrl(),
     * it must reimplement this to open the local file.
     * The default implementation simply returns false.
     *
     * If this method returns true the part emits completed(),
     * otherwise it emits canceled().
     *
     * @see completed(), canceled()
     */
    virtual bool openFile();

    /**
     * @internal
     */
    void abortLoad();

    /**
     * Reimplemented from Part, so that the window caption is set to
     * the current URL (decoded) when the part is activated.
     * This is the usual behavior in 99% of applications.
     * Reimplement if you don't like it - test for event->activated()!
     *
     * @note This is done with GUIActivateEvent and not with
     * PartActivateEvent because it's handled by the main window
     * (which gets the event after the PartActivateEvent events have been sent).
     */
    void guiActivateEvent(GUIActivateEvent *event) override;

    /**
     * Sets the URL associated with this part.
     */
    void setUrl(const QUrl &url);

    /**
     * Returns the local file path associated with this part.
     *
     * @note The result will only be valid if openUrl() or
     * setLocalFilePath() has previously been called.
     */
    QString localFilePath() const;

    /**
     * Sets the local file path associated with this part.
     */
    void setLocalFilePath(const QString &localFilePath);

protected:
    KPARTS_NO_EXPORT ReadOnlyPart(ReadOnlyPartPrivate &dd, QObject *parent);

private:
    Q_DISABLE_COPY(ReadOnlyPart)
};

} // namespace

#endif
