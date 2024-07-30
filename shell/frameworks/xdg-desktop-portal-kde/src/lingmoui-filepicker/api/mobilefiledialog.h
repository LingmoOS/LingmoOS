// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QObject>
#include <QStringList>

class QQmlApplicationEngine;
class FileChooserQmlCallback;
class QQuickWindow;

class MobileFileDialog : public QObject
{
    Q_OBJECT

public:
    MobileFileDialog(QObject *parent);
    ~MobileFileDialog() override = default;

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

    QList<QUrl> results() const;

    uint exec();

Q_SIGNALS:
    void accepted(const QList<QUrl> &files);
    void titleChanged();
    void selectMultipleChanged();
    void selectExistingChanged();
    void nameFiltersChanged();
    void mimeTypeFiltersChanged();
    void folderChanged();
    void currentFileChanged();
    void acceptLabelChanged();
    void selectFolderChanged();
    void cancel();

private:
    QQmlApplicationEngine *m_engine;
    FileChooserQmlCallback *m_callback;
    QList<QUrl> m_results;
    QQuickWindow *m_window;

    bool m_customTitleSet;
};
