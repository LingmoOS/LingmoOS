// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QObject>
#include <QUrl>

class FileChooserQmlCallback : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(bool selectMultiple READ selectMultiple WRITE setSelectMultiple NOTIFY selectMultipleChanged)
    Q_PROPERTY(bool selectExisting READ selectExisting WRITE setSelectExisting NOTIFY selectExistingChanged)
    Q_PROPERTY(QStringList nameFilters READ nameFilters WRITE setNameFilters NOTIFY nameFiltersChanged)
    Q_PROPERTY(QStringList mimeTypeFilters READ mimeTypeFilters WRITE setMimeTypeFilters NOTIFY mimeTypeFiltersChanged)
    Q_PROPERTY(QUrl folder READ folder WRITE setFolder NOTIFY folderChanged)
    Q_PROPERTY(QString currentFile READ currentFile WRITE setCurrentFile NOTIFY currentFileChanged)
    Q_PROPERTY(QString acceptLabel READ acceptLabel WRITE setAcceptLabel NOTIFY acceptLabelChanged)
    Q_PROPERTY(bool selectFolder READ selectFolder WRITE setSelectFolder NOTIFY selectFolderChanged)

public:
    FileChooserQmlCallback(QObject *parent = nullptr);

    QString title() const;
    void setTitle(const QString &title);

    bool selectMultiple() const;
    void setSelectMultiple(bool selectMultiple);

    bool selectExisting() const;
    void setSelectExisting(bool selectExisting);

    QStringList nameFilters() const;
    void setNameFilters(const QStringList &nameFilters);

    QStringList mimeTypeFilters() const;
    void setMimeTypeFilters(const QStringList &mimeTypeFilters);

    QUrl folder() const;
    void setFolder(const QUrl &folder);

    QString currentFile() const;
    void setCurrentFile(const QString &currentFile);

    QString acceptLabel() const;
    void setAcceptLabel(const QString &acceptLabel);

    bool selectFolder() const;
    void setSelectFolder(bool selectFolder);

Q_SIGNALS:
    void accepted(const QList<QUrl> &files);
    void cancel();

    void titleChanged();
    void selectMultipleChanged();
    void selectExistingChanged();
    void nameFiltersChanged();
    void mimeTypeFiltersChanged();
    void folderChanged();
    void currentFileChanged();
    void acceptLabelChanged();
    void selectFolderChanged();

private:
    QString m_title;
    bool m_selectMultiple;
    bool m_selectExisting;
    QStringList m_nameFilters;
    QStringList m_mimeTypeFilters;
    QUrl m_folder;
    QString m_currentFile;
    QString m_acceptLabel;
    bool m_selectFolder;
};
