/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KDEPLATFORMFILEDIALOGBASE_H
#define KDEPLATFORMFILEDIALOGBASE_H

#include <QDialog>
#include <QUrl>

class QDialogButtonBox;
class KDEPlatformFileDialogBase : public QDialog
{
    Q_OBJECT
public:
    friend class KDEPlatformFileDialogHelper;

    explicit KDEPlatformFileDialogBase();
    virtual QUrl directory() = 0;
    virtual void selectMimeTypeFilter(const QString &filter) = 0;
    virtual void selectNameFilter(const QString &filter) = 0;
    virtual void setDirectory(const QUrl &directory) = 0;
    virtual void selectFile(const QUrl &filename) = 0;
    virtual QString selectedMimeTypeFilter() = 0;
    virtual QString selectedNameFilter() = 0;
    virtual QString currentFilterText() = 0;
    virtual QList<QUrl> selectedFiles() = 0;

Q_SIGNALS:
    void closed();
    void fileSelected(const QUrl &file);
    void filesSelected(const QList<QUrl> &files);
    void currentChanged(const QUrl &path);
    void directoryEntered(const QUrl &directory);
    void filterSelected(const QString &filter);

protected:
    void closeEvent(QCloseEvent *e) override;
    QDialogButtonBox *m_buttons = nullptr;
};

#endif
