/*
    SPDX-FileCopyrightText: 2001 Michael Jarrett <michaelj@corel.com>
    SPDX-FileCopyrightText: 2001 Carsten Pfeiffer <pfeiffer@kde.org>
    SPDX-FileCopyrightText: 2009 Shaun Reich <shaun.reich@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDIRSELECTDIALOG_H
#define KDIRSELECTDIALOG_H

#include "kdeplatformfiledialogbase_p.h"
#include <QUrl>

class QAbstractItemView;

/**
 * A pretty dialog for a KDirSelect control for selecting directories.
 * @author Michael Jarrett <michaelj@corel.com>
 */
class KDirSelectDialog : public KDEPlatformFileDialogBase
{
    Q_OBJECT

public:
    /**
     * Creates a new directory selection dialog.
     * @internal use the static selectDirectory function
     * @param startDir the directory, initially shown
     * @param localOnly unused. You can only select paths below the startDir
     * @param parent the parent for the dialog, usually 0L
     */
    explicit KDirSelectDialog(const QUrl &startDir = QUrl(), bool localOnly = false, QWidget *parent = nullptr);

    /**
     * Destroys the directory selection dialog.
     */
    ~KDirSelectDialog() override;

    /**
     * Returns the currently selected URL, or an empty one if no item is selected.
     *
     * If the URL entered in the combobox is valid and exists, it is returned.
     * Otherwise, the URL selected in the treeview is returned instead.
     */
    QUrl url() const;

    /**
     * Returns the root url
     */
    QUrl rootUrl() const;

    /**
     * Returns a pointer to the view which is used for displaying the directories.
     */
    QAbstractItemView *view() const;

    /**
     * Returns whether only local directories can be selected.
     */
    bool localOnly() const;

    /**
     * Creates a KDirSelectDialog, and returns the result.
     * @param startDir the directory, initially shown
     * The tree will display this directory and subdirectories of it.
     * @param localOnly unused. You can only select paths below the startDir
     * @param parent the parent widget to use for the dialog, or NULL to create a parent-less dialog
     * @param caption the caption to use for the dialog, or QString() for the default caption
     * @return The URL selected, or an empty URL if the user canceled
     * or no URL was selected.
     *
     * @deprecated since 5.0, use QFileDialog::getExistingDirectory (if localOnly was true)
     * or QFileDialog::getExistingDirectoryUrl (if localOnly was false) instead.
     */
    static QUrl selectDirectory(const QUrl &startDir = QUrl(), bool localOnly = false, QWidget *parent = nullptr, const QString &caption = QString());

    /**
     * @return The path for the root node
     */
    QUrl startDir() const;

    QUrl directory() override;
    void selectMimeTypeFilter(const QString &filter) override;
    void selectNameFilter(const QString &filter) override;
    void setDirectory(const QUrl &directory) override;
    void selectFile(const QUrl &filename) override;
    QString selectedMimeTypeFilter() override;
    QString selectedNameFilter() override;
    QString currentFilterText() override;
    QList<QUrl> selectedFiles() override;

    void setOkButtonText(const QString &text);
    void setCancelButtonText(const QString &text);
public Q_SLOTS:
    /**
     * Sets the current @p url in the dialog.
     */
    void setCurrentUrl(const QUrl &url);

protected:
    void accept() override;

    /**
     * Reimplemented for saving the dialog geometry.
     */
    void hideEvent(QHideEvent *event) override;

private:
    class Private;
    Private *const d;
};

#endif
