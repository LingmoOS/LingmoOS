/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KDEPLATFORMFILEDIALOGHELPER_H
#define KDEPLATFORMFILEDIALOGHELPER_H

#include "kdeplatformfiledialogbase_p.h"
#include <qpa/qplatformdialoghelper.h>

class KFileWidget;
class KDEPlatformFileDialog : public KDEPlatformFileDialogBase
{
    Q_OBJECT
public:
    friend class KDEPlatformFileDialogHelper;

    explicit KDEPlatformFileDialog();
    QUrl directory() override;
    void selectMimeTypeFilter(const QString &filter) override;
    void selectNameFilter(const QString &filter) override;
    void setDirectory(const QUrl &directory) override;
    void selectFile(const QUrl &filename) override;
    void setViewMode(QFileDialogOptions::ViewMode view);
    void setFileMode(QFileDialogOptions::FileMode mode);
    void setCustomLabel(QFileDialogOptions::DialogLabel label, const QString &text);
    QString selectedMimeTypeFilter() override;
    QString selectedNameFilter() override;
    QString currentFilterText() override;
    QList<QUrl> selectedFiles() override;

protected:
    KFileWidget *const m_fileWidget;
};

class KDEPlatformFileDialogHelper : public QPlatformFileDialogHelper
{
    Q_OBJECT
public:
    KDEPlatformFileDialogHelper();
    ~KDEPlatformFileDialogHelper() override;

    void initializeDialog();

    bool defaultNameFilterDisables() const override;
    QUrl directory() const override;
    QList<QUrl> selectedFiles() const override;
    QString selectedMimeTypeFilter() const override;
    void selectMimeTypeFilter(const QString &filter) override;
    QString selectedNameFilter() const override;
    void selectNameFilter(const QString &filter) override;
    void selectFile(const QUrl &filename) override;
    void setFilter() override;
    void setDirectory(const QUrl &directory) override;
    bool isSupportedUrl(const QUrl &url) const override;

    void exec() override;
    void hide() override;
    bool show(Qt::WindowFlags windowFlags, Qt::WindowModality windowModality, QWindow *parent) override;

    QVariant styleHint(StyleHint hint) const override;

private Q_SLOTS:
    void saveSize();

private:
    QString qt2KdeFilter(const QStringList &f);
    void restoreSize();
    KDEPlatformFileDialogBase *m_dialog = nullptr;
    bool m_directorySet = false;
    bool m_fileSelected = false;
    bool m_dialogInitialized = false;
};

#endif // KDEPLATFORMFILEDIALOGHELPER_H
