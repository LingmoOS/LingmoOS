/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef FILEOPERATIONPROGRESSWIZARD_H
#define FILEOPERATIONPROGRESSWIZARD_H

#include <QWizard>

#include "explor-core_global.h"

class QLabel;
class QProgressBar;

class QFormLayout;
class QGridLayout;
class QSystemTrayIcon;

namespace Peony {

class FileOperationPreparePage;
class FileOperationProgressPage;
class FileOperationAfterProgressPage;
class FileOperationRollbackPage;

/*!
 * \brief The FileOperationProgressWizard class
 * <br>
 * This class is an graphic user interface of serveral file operations.
 * There are two page, the preparing page and the progress page.
 * The preparing page is used to count the source files need to be handled.
 * And the progress page is used to show the current progress of the operation.
 * </br>
 * \note
 * This is the common interface of all kinds of file operation. If you want to
 * implement a special interface for one kind operation. you can dervied the class
 * or re-write a new interface and re-implement the process connect to the operation.
 */
class PEONYCORESHARED_EXPORT FileOperationProgressWizard : public QWizard
{
    friend class FileOperationPreparePage;
    friend class FileOperationProgressPage;
    friend class FileOperationAfterProgressPage;
    friend class FileOperationRollbackPage;
    Q_OBJECT
public:
    enum PageId {
        PreparePage,
        ProgressPage
    };

    explicit FileOperationProgressWizard(QWidget *parent = nullptr);
    ~FileOperationProgressWizard() override;

Q_SIGNALS:
    void cancelled();

public Q_SLOTS:
    virtual void delayShow();

    virtual void switchToPreparedPage();
    virtual void onElementFoundOne(const QString &uri, const qint64 &size);
    virtual void onElementFoundAll();

    virtual void switchToProgressPage();
    virtual void onFileOperationProgressedOne(const QString &uri, const QString &destUri, const qint64 &size);
    virtual void onFileOperationProgressedAll();

    virtual void switchToAfterProgressPage();
    virtual void onElementClearOne(const QString &uri);

    virtual void switchToRollbackPage();
    virtual void onFileRollbacked(const QString &destUri, const QString &srcUri);

    virtual void updateProgress(const QString &srcUri, const QString &destUri, quint64 current, quint64 total);

    virtual void onStartSync();

protected:
    void closeEvent(QCloseEvent *e) override;

    qint64 m_total_size = 0;
    int m_current_size = 0;
    int m_total_count = 0;
    int m_current_count = 1;

    FileOperationPreparePage *m_first_page = nullptr;
    FileOperationProgressPage *m_second_page = nullptr;
    FileOperationAfterProgressPage *m_third_page = nullptr;
    FileOperationRollbackPage *m_last_page = nullptr;

private:
    QSystemTrayIcon *m_tray_icon = nullptr;
    QTimer *m_delayer;
    QTimer *m_tip_delayer;
};

class PEONYCORESHARED_EXPORT FileOperationPreparePage : public QWizardPage
{
    friend class FileOperationProgressWizard;
    Q_OBJECT
public:
    explicit FileOperationPreparePage(QWidget *parent = nullptr);
    ~FileOperationPreparePage();

Q_SIGNALS:
    void cancelled();

private:
    QFormLayout *m_layout = nullptr;
    QLabel *m_src_line = nullptr;
    QLabel *m_state_line = nullptr;
};

class PEONYCORESHARED_EXPORT FileOperationProgressPage : public QWizardPage
{
    friend class FileOperationProgressWizard;
    Q_OBJECT
public:
    explicit FileOperationProgressPage(QWidget *parent = nullptr);
    ~FileOperationProgressPage();

Q_SIGNALS:
    void cancelled();

private:
    QGridLayout *m_layout = nullptr;
    QLabel *m_src_line = nullptr;
    QLabel *m_dest_line = nullptr;
    QLabel *m_state_line = nullptr;
    QProgressBar *m_progress_bar = nullptr;
};

class PEONYCORESHARED_EXPORT FileOperationAfterProgressPage : public QWizardPage
{
    friend class FileOperationProgressWizard;
    Q_OBJECT
public:
    explicit FileOperationAfterProgressPage(QWidget *parent = nullptr);
    ~FileOperationAfterProgressPage();

private:
    QGridLayout *m_layout = nullptr;

    QLabel *m_src_line = nullptr;
    QProgressBar *m_progress_bar = nullptr;

    int m_file_deleted_count = 0;
};

class PEONYCORESHARED_EXPORT FileOperationRollbackPage : public QWizardPage
{
    Q_OBJECT
    friend class FileOperationProgressWizard;
public:
    explicit FileOperationRollbackPage(QWidget *parent = nullptr);
    ~FileOperationRollbackPage();

private:
    QGridLayout *m_layout = nullptr;
    QProgressBar *m_progress_bar = nullptr;

    int m_current_count = 0;
};

}

#endif // FILEOPERATIONPROGRESSWIZARD_H
