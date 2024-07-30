//  -*- c-basic-offset:4; indent-tabs-mode:nil -*-
/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000, 2006 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/
#ifndef __kbookmarkmanager_h
#define __kbookmarkmanager_h

#include <QDomDocument>
#include <QObject>
#include <QString>

#include <memory>

class KBookmarkManagerPrivate;

#include "kbookmark.h"

class KBookmarkGroup;

/**
 * @class KBookmarkManager kbookmarkmanager.h KBookmarkManager
 *
 * This class implements the reading/writing of bookmarks in XML.
 * The bookmarks file is read and written using the XBEL standard
 * (http://pyxml.sourceforge.net/topics/xbel/)
 *
 * A sample file looks like this :
 * \code
 * <xbel>
 *   <bookmark href="http://techbase.kde.org"><title>Developer Web Site</title></bookmark>
 *   <folder folded="no">
 *     <title>Title of this folder</title>
 *     <bookmark icon="kde" href="http://www.kde.org"><title>KDE Web Site</title></bookmark>
 *     <folder toolbar="yes">
 *       <title>My own bookmarks</title>
 *       <bookmark href="http://www.koffice.org"><title>KOffice Web Site</title></bookmark>
 *       <separator/>
 *       <bookmark href="http://www.kdevelop.org"><title>KDevelop Web Site</title></bookmark>
 *     </folder>
 *   </folder>
 * </xbel>
 * \endcode
 */
class KBOOKMARKS_EXPORT KBookmarkManager : public QObject
{
    Q_OBJECT

public:
    /**
     * Create a KBookmarkManager responsible for the given @p bookmarksFile.
     *
     * The manager watches the file for change detection.
     *
     * @param bookmarksFile full path to the bookmarks file,
     * Use ~/.kde/share/apps/konqueror/bookmarks.xml for the konqueror bookmarks
     *
     * @since 6.0
     *
     */
    explicit KBookmarkManager(const QString &bookmarksFile, QObject *parent = nullptr);

    /**
     * Destructor
     */
    ~KBookmarkManager() override;

    /**
     * Save the bookmarks to the given XML file on disk.
     * @param filename full path to the desired bookmarks file location
     * @param toolbarCache iff true save a cache of the toolbar folder, too
     * @return true if saving was successful
     */
    // KF6 TODO: Use an enum and not a bool
    bool saveAs(const QString &filename, bool toolbarCache = true) const;

    /**
     * Update access time stamps for a given url.
     * @param url the viewed url
     * @return true if any metadata was modified (bookmarks file is not saved automatically)
     */
    bool updateAccessMetadata(const QString &url);

    /**
     * This will return the path that this manager is using to read
     * the bookmarks.
     * @internal
     * @return the path containing the bookmarks
     */
    QString path() const;

    /**
     * This will return the root bookmark.  It is used to iterate
     * through the bookmarks manually.  It is mostly used internally.
     *
     * @return the root (top-level) bookmark
     */
    KBookmarkGroup root() const;

    /**
     * This returns the root of the toolbar menu.
     * In the XML, this is the group with the attribute toolbar=yes
     *
     * @return the toolbar group
     */
    KBookmarkGroup toolbar();

    /**
     * @return the bookmark designated by @p address
     * @param address the address belonging to the bookmark you're looking for
     * @param tolerate when true tries to find the most tolerable bookmark position
     * @see KBookmark::address
     */
    KBookmark findByAddress(const QString &address);

    /**
     * Saves the bookmark file and notifies everyone.
     *
     **/
    void emitChanged();

    /**
     * Saves the bookmark file and notifies everyone.
     * @param group the parent of all changed bookmarks
     */
    void emitChanged(const KBookmarkGroup &group);

    /**
     * Save the bookmarks to an XML file on disk.
     * You should use emitChanged() instead of this function, it saves
     * and notifies everyone that the file has changed.
     * Only use this if you don't want the emitChanged signal.
     * @param toolbarCache iff true save a cache of the toolbar folder, too
     * @return true if saving was successful
     */
    // KF6 TODO: Use an enum and not a bool
    bool save(bool toolbarCache = true) const;

    /**
     * @internal
     */
    QDomDocument internalDocument() const;

Q_SIGNALS:
    /**
     * Signals that the group (or any of its children) with the address
     * @p groupAddress (e.g. "/4/5")
     * has been modified.
     * connect to this
     */
    void changed(const QString &groupAddress);

    /**
     * Emitted when an error occurs.
     * Contains the translated error message.
     * @since 4.6
     */
    void error(const QString &errorMessage);

private Q_SLOTS:
    KBOOKMARKS_NO_EXPORT void slotFileChanged(const QString &path); // external bookmarks

private:
    // consts added to avoid a copy-and-paste of internalDocument
    KBOOKMARKS_NO_EXPORT void parse() const;

    KBOOKMARKS_NO_EXPORT void startKEditBookmarks(const QStringList &args);

private:
    std::unique_ptr<KBookmarkManagerPrivate> const d;

    friend class KBookmarkGroup;
};

#endif
