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

#include "file-operation-progress-wizard.h"

#include <QFormLayout>
#include <QGridLayout>

#include <QLabel>
#include <QProgressBar>

#include <QPushButton>

#include <QCloseEvent>
#include <QMessageBox>

#include <QSystemTrayIcon>

#include <QTimer>

#include <gio/gio.h>

#include <QDebug>

#include <QDesktopWidget>
#include <QApplication>

using namespace Peony;

FileOperationProgressWizard::FileOperationProgressWizard(QWidget *parent) : QWizard(parent)
{
    setFixedWidth(600);
    setFixedHeight(480);

    setWindowFlags(windowFlags());
    setWindowTitle(tr("File Manager"));
    //only show cancel button at bottom-right of wizard
    QList<WizardButton> layout;
    layout<<Stretch<<CustomButton1;
    setButtonText(CustomButton1, tr("&Cancel"));
    connect(this, &QWizard::customButtonClicked, [=]() {
        this->cancelled();
    });
    setButtonLayout(layout);

    //connect(this, &QDialog::rejected, this, &FileOperationProgressWizard::cancelled);

    m_first_page = new FileOperationPreparePage(this);
    m_first_page->setTitle(tr("Preparing..."));
    addPage(m_first_page);
    m_second_page = new FileOperationProgressPage(this);
    m_second_page->setTitle(tr("Handling..."));
    addPage(m_second_page);
    m_third_page = new FileOperationAfterProgressPage(this);
    m_third_page->setTitle(tr("Clearing..."));
    addPage(m_third_page);
    m_last_page = new FileOperationRollbackPage(this);
    m_last_page->setTitle(tr("Rollbacking..."));
    addPage(m_last_page);

    m_tray_icon = new QSystemTrayIcon(QIcon::fromTheme("system-file-manager"), this);
    m_tray_icon->setToolTip(tr("File Operation"));

    connect(m_tray_icon, &QSystemTrayIcon::activated, [=]() {
        this->show();
        m_tray_icon->hide();
    });

    m_delayer = new QTimer(this);
    m_tip_delayer = new QTimer(this);
    m_delayer->setSingleShot(true);
    m_delayer->setInterval(100);

    connect(m_tip_delayer, &QTimer::timeout, [=]() {
    m_tray_icon->showMessage(tr("File Operation"),
                                 tr("A file operation is running backend..."),
    #if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
                                 QIcon::fromTheme("system-file-manager"),
    #else
                                 QSystemTrayIcon::MessageIcon::Information,
    #endif
                                 5000);
    m_tip_delayer->stop();
    });
}

FileOperationProgressWizard::~FileOperationProgressWizard()
{

}

void FileOperationProgressWizard::closeEvent(QCloseEvent *e)
{
    //NOTE: the wizard will destroy when file operation finished.
    //ignore the close event and just hide itself.
    //FIXME: the close button bolder-style will changed if closeEvent
    //was overwrite. is that a bug?
    e->ignore();
    m_tray_icon->show();

    m_tray_icon->showMessage(tr("File Operation"),
                             tr("A file operation is running backend..."),
#if (QT_VERSION >= QT_VERSION_CHECK(5, 9, 0))
                             QIcon::fromTheme("system-file-manager"),
#else
                             QSystemTrayIcon::MessageIcon::Information,
#endif
                             1);

    m_tip_delayer->start(800);

    hide();
}

void FileOperationProgressWizard::delayShow()
{
    QTimer::singleShot(1000, this, &FileOperationProgressWizard::show);
}

void FileOperationProgressWizard::switchToPreparedPage()
{
    restart();

    m_total_count = 0;
    m_total_size = 0;
    auto cancelButton = button(QWizard::CustomButton1);
    cancelButton->setEnabled(true);
}

void FileOperationProgressWizard::onElementFoundOne(const QString &uri, const qint64 &size)
{
    qDebug()<<"onElementFound"<<uri<<size;
    m_total_count++;
    m_total_size += size;

    //char *format_size = g_format_size (quint64(m_total_size));
    //Calculated by 1024 bytes
    char *format_size = strtok(g_format_size_full(quint64(m_total_size),G_FORMAT_SIZE_IEC_UNITS),"iB");

    m_first_page->m_src_line->setText(uri);
    m_first_page->m_state_line->setText(tr("%1 files, %2").arg(m_total_count).arg(format_size));

    g_free(format_size);
}

void FileOperationProgressWizard::onElementFoundAll()
{
    switchToProgressPage();
}

void FileOperationProgressWizard::switchToProgressPage()
{
    restart();
    next();

    auto cancelButton = button(QWizard::CustomButton1);
    cancelButton->setEnabled(true);
}

void FileOperationProgressWizard::onFileOperationProgressedOne(const QString &uri, const QString &destUri, const qint64 &size)
{
    m_current_count++;
    return;
}

void FileOperationProgressWizard::onFileOperationProgressedAll()
{
    switchToAfterProgressPage();
}

void FileOperationProgressWizard::switchToAfterProgressPage()
{
    restart();
    next();
    next();

    auto cancelButton = button(QWizard::CustomButton1);
    cancelButton->setEnabled(true);
}

void FileOperationProgressWizard::onElementClearOne(const QString &uri)
{
    m_third_page->m_file_deleted_count++;
    m_third_page->m_src_line->setText(tr("clearing: %1, %2 of %3").arg(uri).
                                      arg(m_third_page->m_file_deleted_count).
                                      arg(m_total_count));

    if (m_total_count > 0)
        m_third_page->m_progress_bar->setValue(int(m_third_page->m_file_deleted_count*100.0)/m_total_count);

}

void FileOperationProgressWizard::switchToRollbackPage()
{
    restart();
    next();
    next();
    next();

    //rollback is not cancellable, so disable cancel button.
    auto cancelButton = button(QWizard::CustomButton1);
    cancelButton->setEnabled(false);
}

void FileOperationProgressWizard::onFileRollbacked(const QString &destUri, const QString &srcUri)
{
    Q_UNUSED(destUri);
    Q_UNUSED(srcUri);
    m_last_page->m_current_count++;
    auto c = m_last_page->m_current_count;
    auto t = m_current_count;
    auto v = qreal(c*1.0/t)*100;
    //use wizard's m_current_count as total count of files need rollback.
    m_last_page->m_progress_bar->setValue(int(v));
}

void FileOperationProgressWizard::updateProgress(const QString &srcUri, const QString &destUri, quint64 current, quint64 total)
{
    if (m_delayer->isActive()) {
        return;
    }

    if (current > m_total_size)
        return;

    m_delayer->start();

    if (m_second_page->m_state_line->text() == "unknown") {
        m_second_page->m_state_line->setText(tr("copying..."));
    }
    m_second_page->m_src_line->setText(srcUri);
    m_second_page->m_dest_line->setText(destUri);

    //char *current_format_size = g_format_size (quint64(current));
    //char *total_format_size = g_format_size(quint64(m_total_size));

    //Calculated by 1024 bytes
    char *current_format_size = strtok(g_format_size_full(quint64(current),G_FORMAT_SIZE_IEC_UNITS),"iB");
    char *total_format_size = strtok(g_format_size_full(quint64(m_total_size),G_FORMAT_SIZE_IEC_UNITS),"iB");

    m_second_page->m_state_line->setText(tr("%1 done, %2 total, %3 of %4.").
                                         arg(current_format_size).arg(total_format_size)
                                         .arg(m_current_count).arg(m_total_count));

    g_free(current_format_size);
    g_free(total_format_size);
    m_second_page->m_src_line->setText(srcUri);
    m_second_page->m_dest_line->setText(destUri);
    double test = (m_current_size*1.0/m_total_size)*100;
    m_second_page->m_progress_bar->setValue(int(test));

    double progress = current*1.0/total;
    m_second_page->m_progress_bar->setValue(int(progress*100));
}

void FileOperationProgressWizard::onStartSync()
{
    switchToAfterProgressPage();
    m_third_page->setTitle(tr("Syncing..."));
    m_third_page->m_src_line->hide();
    // set a busy progress bar
    m_third_page->m_progress_bar->setMaximum(0);
    m_third_page->m_progress_bar->setMinimum(0);

    //QMessageBox::information(0, 0, "syncing");
}

//FileOperationPreparePage
FileOperationPreparePage::FileOperationPreparePage(QWidget *parent) : QWizardPage (parent)
{
    m_layout = new QFormLayout(this);

    m_layout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    m_layout->setLabelAlignment(Qt::AlignRight);
    m_layout->setFormAlignment(Qt::AlignLeft);

    m_src_line = new QLabel("null", this);
    m_state_line = new QLabel("0 files, 0 bytes", this);
    m_layout->addRow(tr("counting:"), m_src_line);
    m_layout->addRow(tr("state:"), m_state_line);

    setLayout(m_layout);
}

FileOperationPreparePage::~FileOperationPreparePage()
{

}

//FileOperationProgressPage
FileOperationProgressPage::FileOperationProgressPage(QWidget *parent) : QWizardPage (parent)
{
    m_layout = new QGridLayout(this);

    m_state_line = new QLabel("unknown", this);

    m_progress_bar = new QProgressBar(this);

    m_layout->addWidget(m_state_line, 0, 0);
    m_layout->addWidget(m_progress_bar, 1, 0);

    QPushButton *details_button = new QPushButton(tr("&More Details"), this);
    details_button->setCheckable(true);
    m_layout->addWidget(details_button, 2, 0, Qt::AlignRight);

    QFormLayout *moreDetailsLayout = new QFormLayout;
    moreDetailsLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    moreDetailsLayout->setLabelAlignment(Qt::AlignRight);
    moreDetailsLayout->setFormAlignment(Qt::AlignLeft);

    m_src_line = new QLabel("null", this);
    m_dest_line = new QLabel("null", this);
    moreDetailsLayout->addRow(tr("From:"), m_src_line);
    moreDetailsLayout->addRow(tr("To:"), m_dest_line);

    QWidget *detailsWidget = new QWidget(this);
    detailsWidget->setLayout(moreDetailsLayout);
    m_layout->addWidget(detailsWidget, 3, 0);
    detailsWidget->hide();
    connect(details_button, &QAbstractButton::toggled, detailsWidget, &QWidget::setVisible);

    setLayout(m_layout);
}

FileOperationProgressPage::~FileOperationProgressPage()
{

}

//FileOperationAfterProgressPage
FileOperationAfterProgressPage::FileOperationAfterProgressPage(QWidget *parent) : QWizardPage (parent)
{
    m_layout = new QGridLayout(this);

    m_src_line = new QLabel("clearing: null", this);
    //avoid wizard size hint changed.
    m_src_line->setWordWrap(true);
    m_src_line->setVisible(false);
    m_progress_bar = new QProgressBar(this);

    m_layout->addWidget(m_progress_bar, 0, 0);

    QPushButton *details_button = new QPushButton(tr("&More Details"), this);
    details_button->setCheckable(true);
    m_layout->addWidget(details_button, 1, 0, Qt::AlignRight);
    m_layout->addWidget(m_src_line, 2, 0);
    details_button->setVisible(false);
    m_src_line->hide();
    connect(details_button, &QAbstractButton::toggled, m_src_line, &QLabel::setVisible);

    setLayout(m_layout);
}

FileOperationAfterProgressPage::~FileOperationAfterProgressPage()
{

}

//FileOperationRollbackPage
FileOperationRollbackPage::FileOperationRollbackPage(QWidget *parent) : QWizardPage (parent)
{
    m_layout = new QGridLayout(this);
    m_progress_bar = new QProgressBar(this);
    m_layout->addWidget(m_progress_bar, 0, 0);

    setLayout(m_layout);
}

FileOperationRollbackPage::~FileOperationRollbackPage()
{

}
