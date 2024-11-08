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

#include "recent-and-trash-properties-page.h"

#include "file-info.h"
#include "file-info-job.h"
#include "file-utils.h"
#include "global-settings.h"
#include "file-count-operation.h"
#include "file-meta-info.h"

#include <QGSettings>
#include <QFormLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QUrl>

#include <QCheckBox>
#include <gio/gdesktopappinfo.h>

using namespace Peony;
//460 - 16 - 16 = 428
#define FIXED_ROW_WIDTH 428;

QString RecentAndTrashPropertiesPage::getIconName() {
    if (m_fileInfo == nullptr)
        return "application-x-desktop";

    QString realPath;
    realPath = m_fileInfo->targetUri();

    auto _desktop_file = g_desktop_app_info_new_from_filename(QUrl(realPath).path().toUtf8().constData());
    if (_desktop_file) {
        return QString(g_desktop_app_info_get_string(_desktop_file, "Icon"));
    }
    //在找不到图标时，返回默认图标 - When the icon is not found, return to the default icon
    return "application-x-desktop";
}

RecentAndTrashPropertiesPage::RecentAndTrashPropertiesPage(const QStringList &uris, QWidget *parent) : PropertiesWindowTabIface(parent)
{
    m_uri = uris.first();

    m_fileInfo = FileInfo::fromUri(m_uri);
    FileInfoJob *job = new FileInfoJob(m_fileInfo);
    job->setAutoDelete(true);
    connect(job, &FileInfoJob::queryAsyncFinished, this, &RecentAndTrashPropertiesPage::init);
    job->queryAsync();
}

void RecentAndTrashPropertiesPage::init()
{
    if (m_futureWatcher) {
        delete m_futureWatcher;
        m_futureWatcher = nullptr;
    }

    auto targetFileInfo = FileInfo::fromUri(m_fileInfo.get()->targetUri());
    FileInfoJob j(targetFileInfo);
    j.querySync();

    m_layout = new QFormLayout(this);
    m_layout->setRowWrapPolicy(QFormLayout::WrapLongRows);
    m_layout->setFormAlignment(Qt::AlignLeft|Qt::AlignHCenter);
    m_layout->setLabelAlignment(Qt::AlignRight|Qt::AlignHCenter);
    m_layout->setContentsMargins(16,16,16,0);
    this->setLayout(m_layout);

    QString iconName = m_fileInfo->iconName();
    if (iconName == "application-x-desktop") {
        iconName = getIconName();
    }
    auto icon = new QPushButton(QIcon::fromTheme(iconName), nullptr, this);
    icon->setFocusPolicy(Qt::NoFocus);
    icon->setAttribute(Qt::WA_TransparentForMouseEvents);
    icon->setIconSize(QSize(48, 48));
    icon->setProperty("isIcon", true);

    QVBoxLayout *boxLayout = new QVBoxLayout(this);
    auto name = new QLineEdit(this);
    name->setReadOnly(true);
    name->setText(m_fileInfo->displayName());
    if (!targetFileInfo.get()->displayName().isEmpty()) {
        name->setText(targetFileInfo.get()->displayName());
    }

    boxLayout->addWidget(name);
    boxLayout->setAlignment(Qt::AlignBottom);

    m_layout->addRow(icon, boxLayout);
    m_layout->setAlignment(Qt::AlignCenter);

    addSeparator();

    bool startWithTrash = m_uri.startsWith("trash:///");

    if (startWithTrash) {
        if (m_uri == "trash:///") {
            auto checkbox = new QCheckBox(tr("Show confirm dialog while trashing."));
            m_layout->addWidget(checkbox);
            connect(checkbox, &QCheckBox::toggled, this, [=](bool checked){
                this->setProperty("check", checked);
            });
            auto value = GlobalSettings::getInstance()->getValue("showTrashDialog");
            if (value.isValid()) {
                checkbox->setChecked(value.toBool());
            } else {
                checkbox->setChecked(true);
            }
        } else {
            QLabel *label =new QLabel(this);
            GFile *file = g_file_new_for_uri(m_uri.toUtf8().constData());
            GFileInfo *info = g_file_query_info(file,
                                                G_FILE_ATTRIBUTE_TRASH_ORIG_PATH,
                                                G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                                nullptr,
                                                nullptr);
            auto origin_path = g_file_info_get_attribute_byte_string(info, G_FILE_ATTRIBUTE_TRASH_ORIG_PATH);

            if (!origin_path) {
                auto targetInfo = FileInfo::fromUri(m_uri);
                QString path = targetInfo.get()->property("orig-path").toString();
                if (!path.isEmpty()) {
                    origin_path = path.toUtf8().constData();
                }
            }

            QUrl url(FileUtils::getParentUri("file://" + QString(origin_path)));

            quint64 width = FIXED_ROW_WIDTH - label->fontMetrics().width(tr("Origin Path: "));
            label->setText(label->fontMetrics().elidedText(url.path(), Qt::ElideMiddle,width));
            label->setWordWrap(true);

//            g_object_unref(info);
//            g_object_unref(file);
            m_layout->addRow(tr("Origin Path: "), label);

            QLabel *size_label =new QLabel(this);
            if (m_fileInfo->isDir()) {
                FileCountOperation *fileCountOp = new FileCountOperation(QStringList() << m_fileInfo->uri());
                fileCountOp->setAutoDelete(true);

                connect(fileCountOp, &FileCountOperation::countDone, [=](quint64 file_count, quint64 hidden_file_count, quint64 total_size) {
                    char *fileTotalSizeFormat = g_format_size_full(total_size, G_FORMAT_SIZE_IEC_UNITS);

                    QString fileTotalSizeFormatString(fileTotalSizeFormat);

                    size_label->setText(fileTotalSizeFormatString.replace("iB", "B"));

                    g_free(fileTotalSizeFormat);
                });
                QThreadPool::globalInstance()->start(fileCountOp);
            } else {
                size_label->setText(m_fileInfo->fileSize());
            }

            m_layout->addRow(tr("Size: "), size_label);
            //add delete date label
            QLabel *delete_label =new QLabel(this);
            info = g_file_query_info(file,
                                     G_FILE_ATTRIBUTE_TRASH_DELETION_DATE,
                                     G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                     nullptr,
                                     nullptr);

            //use sdk interface to get time format
            QString deletion_date = m_fileInfo->deletionDate();
            if (deletion_date.isEmpty()) {
                deletion_date = m_fileInfo->modifiedDate();
            }
            quint64 delete_width = FIXED_ROW_WIDTH - delete_label->fontMetrics().width(tr("Deletion Date: "));
            delete_label->setText(label->fontMetrics().elidedText(deletion_date, Qt::ElideMiddle, delete_width));
            delete_label->setWordWrap(true);

            //no need of old way
//            if (QGSettings::isSchemaInstalled("org.lingmo.control-center.panel.plugins")) {
//                QGSettings *settings = new QGSettings("org.lingmo.control-center.panel.plugins", "", this);
//                connect(settings, &QGSettings::changed, this, [=](const QString &key) {
//                    if(key == "date") {
//                        QString current_text = delete_label->text();
//                        QString new_date_type = settings->get("date").toString();
//                        //cn : 1999/11/11
//                        //en : 1999-11-11
//                        if ((new_date_type == "cn") && current_text.contains("-")) {
//                            delete_label->setText(current_text.replace("-", "/"));

//                        } else if ((new_date_type == "en") && current_text.contains("/")) {
//                            delete_label->setText(current_text.replace("/", "-"));
//                        }
//                    }
//                });

//                QString current_text = delete_label->text();
//                QString new_date_type = settings->get("date").toString();
//                if ((new_date_type == "cn") && current_text.contains("-")) {
//                    delete_label->setText(current_text.replace("-", "/"));

//                } else if ((new_date_type == "en") && current_text.contains("/")) {
//                    delete_label->setText(current_text.replace("/", "-"));
//                }
//            }

            g_object_unref(info);
            g_object_unref(file);
            m_layout->addRow(tr("Deletion Date: "), delete_label);
        }
    } else {
        if (m_uri == "recent:///") {

        } else {
            QLabel *sizeLabel =new QLabel(this);
            QLabel *locationLabel =new QLabel(this);
            auto targetUri = m_fileInfo->targetUri();
            quint64 width = FIXED_ROW_WIDTH - locationLabel->fontMetrics().width(tr("Original Location: "));
            locationLabel->setText(locationLabel->fontMetrics().elidedText(QUrl(targetUri).path(), Qt::ElideMiddle,width));
            locationLabel->setWordWrap(true);

            sizeLabel->setText(m_fileInfo->fileSize());

            m_layout->addRow(tr("Size: "), sizeLabel);
            m_layout->addRow(tr("Original Location: "), locationLabel);
        }
    }
}

void RecentAndTrashPropertiesPage::addSeparator()
{
    auto separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine);
    m_layout->addRow(separator);
}

void RecentAndTrashPropertiesPage::saveAllChange()
{
    bool check = this->property("check").toBool();
    GlobalSettings::getInstance()->setGSettingValue("showTrashDialog", check);
}
