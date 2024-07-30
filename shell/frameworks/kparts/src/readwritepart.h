/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 1999 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _KPARTS_READWRITEPART_H
#define _KPARTS_READWRITEPART_H

#include <kparts/readonlypart.h>

namespace KParts
{
class ReadWritePartPrivate;

/**
 * @class ReadWritePart readwritepart.h <KParts/ReadWritePart>
 *
 * @short Base class for an "editor" part.
 *
 * This class handles network transparency for you.
 * Anything that can open a URL, allow modifications, and save
 * (to the same URL or a different one).
 *
 * A read-write part can be set to read-only mode, using setReadWrite().
 *
 * Part writers :
 * Any part inheriting ReadWritePart should check isReadWrite
 * before allowing any action that modifies the part.
 * The part probably wants to reimplement setReadWrite, disable those
 * actions. Don't forget to call the parent setReadWrite.
 */
class KPARTS_EXPORT ReadWritePart : public ReadOnlyPart
{
    Q_OBJECT

    KPARTS_DECLARE_PRIVATE(ReadWritePart)

public:
    /**
     * Constructor
     * See parent constructor for instructions.
     */
    explicit ReadWritePart(QObject *parent = nullptr, const KPluginMetaData &data = {});
    /**
     * Destructor
     * Applications using a ReadWritePart should make sure, before
     * destroying it, to call closeUrl().
     * In KMainWindow::queryClose(), for instance, they should allow
     * closing only if the return value of closeUrl() was true.
     * This allows to cancel.
     */
    ~ReadWritePart() override;

    /**
     * @return true if the part is in read-write mode
     */
    bool isReadWrite() const;

    /**
     * Changes the behavior of this part to readonly or readwrite.
     * @param readwrite set to true to enable readwrite mode
     */
    virtual void setReadWrite(bool readwrite = true);

    /**
     * @return true if the document has been modified.
     */
    bool isModified() const;

    /**
     * If the document has been modified, ask the user to save changes.
     * This method is meant to be called from KMainWindow::queryClose().
     * It will also be called from closeUrl().
     *
     * @return true if closeUrl() can be called without the user losing
     * important data, false if the user chooses to cancel.
     */
    virtual bool queryClose();

    /**
     * Called when closing the current url (e.g. document), for instance
     * when switching to another url (note that openUrl() calls it
     * automatically in this case).
     *
     * If the current URL is not fully loaded yet, aborts loading.
     *
     * If isModified(), queryClose() will be called.
     *
     * @return false on cancel
     */
    bool closeUrl() override;

    /**
     * Call this method instead of the above if you need control if
     * the save prompt is shown. For example, if you call queryClose()
     * from KMainWindow::queryClose(), you would not want to prompt
     * again when closing the url.
     *
     * Equivalent to promptToSave ? closeUrl() : ReadOnlyPart::closeUrl()
     */
    virtual bool closeUrl(bool promptToSave);

    /**
     * Save the file to a new location.
     *
     * Calls save(), no need to reimplement
     */
    virtual bool saveAs(const QUrl &url);

    /**
     *  Sets the modified flag of the part.
     */
    virtual void setModified(bool modified);

Q_SIGNALS:
    /**
     * set handled to true, if you don't want the default handling
     * set abortClosing to true, if you handled the request,
     * but for any reason don't  want to allow closing the document
     */
    void sigQueryClose(bool *handled, bool *abortClosing);

public Q_SLOTS:
    /**
     * Call setModified() whenever the contents get modified.
     * This is a slot for convenience, since it simply calls setModified(true),
     * so that you can connect it to a signal, like textChanged().
     */
    void setModified();

    /**
     * Save the file in the location from which it was opened.
     * You can connect this to the "save" action.
     * Calls saveFile() and saveToUrl(), no need to reimplement.
     */
    virtual bool save();

    /**
     * Waits for any pending upload job to finish and returns whether the
     * last save() action was successful.
     */
    bool waitSaveComplete();

protected:
    /**
     * Save to a local file.
     * You need to implement it, to save to the local file.
     * The framework takes care of re-uploading afterwards.
     *
     * @return true on success, false on failure.
     * On failure the function should inform the user about the
     * problem with an appropriate message box. Standard error
     * messages can be constructed using KIO::buildErrorString()
     * in combination with the error codes defined in kio/global.h
     */
    virtual bool saveFile() = 0;

    /**
     * Save the file.
     *
     * Uploads the file, if @p url is remote.
     * This will emit started(), and either completed() or canceled(),
     * in case you want to provide feedback.
     * @return true on success, false on failure.
     */
    virtual bool saveToUrl();

private:
    Q_DISABLE_COPY(ReadWritePart)
};

} // namespace

#endif
