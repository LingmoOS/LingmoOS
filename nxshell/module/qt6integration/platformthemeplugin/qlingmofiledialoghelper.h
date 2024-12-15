/*
 * SPDX-FileCopyrightText: 2017 - 2023 UnionTech Software Technology Co., Ltd.
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */
#ifndef QLINGMOFILEDIALOGHELPER_H
#define QLINGMOFILEDIALOGHELPER_H

#include <qpa/qplatformdialoghelper.h>

#include <QPointer>
#include <QEventLoop>

QT_BEGIN_NAMESPACE

class ComLingmoFilemanagerFiledialogInterface;
typedef ComLingmoFilemanagerFiledialogInterface DFileDialogHandle;
class ComLingmoFilemanagerFiledialogmanagerInterface;
typedef ComLingmoFilemanagerFiledialogmanagerInterface DFileDialogManager;
class QLingmoFileDialogHelper : public QPlatformFileDialogHelper
{
public:
    enum CustomWidgetType {
        LineEditType = 0,
        ComboBoxType = 1
    };

    QLingmoFileDialogHelper();
    ~QLingmoFileDialogHelper();

    bool show(Qt::WindowFlags flags, Qt::WindowModality modality, QWindow *parent) Q_DECL_OVERRIDE;
    void exec() Q_DECL_OVERRIDE;
    void hide() Q_DECL_OVERRIDE;

    bool defaultNameFilterDisables() const Q_DECL_OVERRIDE;
    void setDirectory(const QUrl &directory) Q_DECL_OVERRIDE;
    QUrl directory() const Q_DECL_OVERRIDE;
    void selectFile(const QUrl &filename) Q_DECL_OVERRIDE;
    QList<QUrl> selectedFiles() const Q_DECL_OVERRIDE;
    void setFilter() Q_DECL_OVERRIDE;
    void selectNameFilter(const QString &filter) Q_DECL_OVERRIDE;
    QString selectedNameFilter() const Q_DECL_OVERRIDE;

    static void initDBusFileDialogManager();
    static bool iAmFileDialogDBusServer();
protected:
    void onApplicationStateChanged(Qt::ApplicationState state);
    void onWindowActiveChanged();
private:
    mutable QPointer<DFileDialogHandle> filedlgInterface;
    mutable QPointer<QWindow> auxiliaryWindow;
    QPointer<QWindow> activeWindow;
    QPointer<QObject> sourceDialog;
    QPointer<QEventLoop> execLoop;
    static DFileDialogManager *manager;
    static QString dialogService;

    void ensureDialog() const;
    void applyOptions();
    void hideAuxiliaryWindow() const;

    friend class QLingmoTheme;
};

QT_END_NAMESPACE

#endif // QLINGMOFILEDIALOGHELPER_H
