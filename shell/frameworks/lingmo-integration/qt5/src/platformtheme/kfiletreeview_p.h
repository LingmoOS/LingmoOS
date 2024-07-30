/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KFILETREEVIEW_H
#define KFILETREEVIEW_H

#include <QTreeView>

#include <QUrl>

/**
 * The file treeview offers a treeview on the filesystem.
 */
class KFileTreeView : public QTreeView // exported only for kfiletreeviewtest
{
    Q_OBJECT

public:
    /**
     * Creates a new file tree view.
     */
    explicit KFileTreeView(QWidget *parent = nullptr);

    /**
     * Destroys the file tree view.
     */
    ~KFileTreeView() override;

    /**
     * Returns the current url.
     */
    QUrl currentUrl() const;

    /**
     * Returns the selected url.
     */
    QUrl selectedUrl() const;

    /**
     * Returns all selected urls.
     */
    QList<QUrl> selectedUrls() const;

    /**
     * Returns the current root url of the view.
     */
    QUrl rootUrl() const;

    /**
     * Returns true if the view is currently showing hidden files
     * @since 4.3
     */
    bool showHiddenFiles() const;

    /**
     * @reimplemented
     */
    QSize sizeHint() const override;

public Q_SLOTS:
    /**
     * Sets whether the dir-only mode is @p enabled.
     *
     * In dir-only mode, only directories and subdirectories
     * are listed in the view.
     */
    void setDirOnlyMode(bool enabled);

    /**
     * Sets whether hidden files shall be listed.
     */
    void setShowHiddenFiles(bool enabled);

    /**
     * Sets the current @p url of the view.
     */
    void setCurrentUrl(const QUrl &url);

    /**
     * Sets the root @p url of the view.
     *
     * The default is file:///.
     */
    void setRootUrl(const QUrl &url);

Q_SIGNALS:
    /**
     * This signal is emitted whenever an @p url has been activated.
     */
    void activated(const QUrl &url);

    /**
     * This signal is emitted whenever the current @p url has been changed.
     */
    void currentUrlChanged(const QUrl &url);

protected:
    void contextMenuEvent(QContextMenuEvent *) override;

private:
    class Private;
    Private *const d;
};

#endif
